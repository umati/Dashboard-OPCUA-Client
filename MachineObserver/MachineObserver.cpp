 /* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 * 
 * Copyright 2019-2021 (c) Christian von Arnim, ISW University of Stuttgart (for umati and VDW e.V.)
 * Copyright 2020 (c) Dominik Basner, Sotec GmbH (for VDW e.V.)
 * Copyright 2021 (c) Moritz Walker, ISW University of Stuttgart (for umati and VDW e.V.)
 */

#include "MachineObserver.hpp"
#include <easylogging++.h>
#include <Exceptions/OpcUaException.hpp>
#include <Exceptions/ClientNotConnected.hpp>
#include <Exceptions/MachineOfflineException.hpp>
#include <TypeDefinition/UmatiTypeNodeIds.hpp>
#include <utility>

namespace Umati {
    namespace MachineObserver {

        MachineObserver::MachineObserver(
                std::shared_ptr<Dashboard::IDashboardDataClient> pDataClient,
                std::shared_ptr<Umati::Dashboard::OpcUaTypeReader> pTypeReader,
                std::vector<ModelOpcUa::NodeId_t> machinesFilter
        )
                : m_pDataClient(std::move(pDataClient)), m_pOpcUaTypeReader(std::move(pTypeReader)), m_machinesFilter(machinesFilter.begin(), machinesFilter.end()) {
        }

        MachineObserver::~MachineObserver() {}

        /**
        * index 1000 is the folder machineTools where all the machines are inside
        */
        void MachineObserver::UpdateMachines() {

            /**
            * Assumes that all machines are offline / to be removed
            */
            std::set<ModelOpcUa::NodeId_t> toBeRemovedMachines;
            for(auto & knownMachine: m_knownMachines)
            {
                toBeRemovedMachines.insert(knownMachine.first);
            }
            std::list<ModelOpcUa::BrowseResult_t> machineList;
            /**
            * Browses the machineList and fills the list if possiblenodeClassFromN
            */
            if (!canBrowseMachineList(machineList)) {
                return;
            }
            // Remove duplicate machines
            machineList.sort([](const ModelOpcUa::BrowseResult_t &l, const ModelOpcUa::BrowseResult_t &r)-> bool {
                    return l.NodeId < r.NodeId;
                });
            std::unique(machineList.begin(), machineList.end(),
                [](const ModelOpcUa::BrowseResult_t &l, const ModelOpcUa::BrowseResult_t &r) -> bool{
                    return l.NodeId == r.NodeId;
                });

            std::map<ModelOpcUa::NodeId_t, ModelOpcUa::BrowseResult_t> machineList_map;
            std::transform(machineList.begin(), machineList.end(),
                std::inserter(machineList_map, machineList_map.end()),
                    [](const ModelOpcUa::BrowseResult_t &m) { return std::make_pair(m.NodeId, m); }
            );

            machineListsNotEqual(machineList);
            std::set<ModelOpcUa::NodeId_t> newMachines;
            std::map<ModelOpcUa::NodeId_t, nlohmann::json> machinesIdentification;
            findNewAndOfflineMachines(machineList, toBeRemovedMachines, newMachines, machinesIdentification);

            removeOfflineMachines(toBeRemovedMachines);

            for (auto &newMachineNodeId : newMachines) {
                // Ignore known invalid machines for a specific time
                if (ignoreInvalidMachinesTemporarily(newMachineNodeId)) {
                    continue;
                };
                addNewMachine(machineList_map[newMachineNodeId]);
            }

            {
                std::unique_lock<decltype(m_machineIdentificationsCache_mutex)> ul(m_machineIdentificationsCache_mutex);
                m_machineIdentificationsCache = machinesIdentification;
            }

        }

        bool MachineObserver::machineListsNotEqual(std::list<ModelOpcUa::BrowseResult_t> &machineList) {
            /// \TODO Is this function still required? Is this handled by the reset logic?

            std::set<ModelOpcUa::NodeId_t> newMachines;
            // Use a set for a quick compare, as there might be duplicates in machineList!
            for (const auto &machineTool : machineList) {
                newMachines.insert(machineTool.NodeId);
            }
            if(newMachines != m_knownMachineToolsSet)
            {
                LOG(INFO) << "Different set of machines, reset known machines.";
                recreateKnownMachineToolsMap(machineList);
                return true;
            }
            return false;
        }

        void MachineObserver::recreateKnownMachineToolsMap(std::list<ModelOpcUa::BrowseResult_t> &machineList) {
            LOG(WARNING) << "Lists differ, recreating known machine tools map";
            removeOfflineMachines(m_knownMachineToolsSet);
            m_knownMachineToolsSet.clear();
            for (const auto &machineTool : machineList) {
                m_knownMachineToolsSet.insert(machineTool.NodeId);
            }
        }

        bool MachineObserver::ignoreInvalidMachinesTemporarily(const ModelOpcUa::NodeId_t &newMachineId) {
            auto it = m_invalidMachines.find(newMachineId);
            if (it != m_invalidMachines.end()) {
                --(it->second.first);
                if (it->second.first <= 0) {
                    m_invalidMachines.erase(it); // todo or does it need to be it++?
                } else {
                    return true;
                }
            }
            return false;
        }

        bool MachineObserver::canBrowseMachineList(std::list<ModelOpcUa::BrowseResult_t> &machineList) {
            try {
                LOG(INFO) << "Searching for machines";
                machineList.empty();
                std::function<bool(ModelOpcUa::NodeId_t)> filter;
                if(m_machinesFilter.size()){
                    filter = [&](ModelOpcUa::NodeId_t machine){
                        return m_machinesFilter.find(machine) != m_machinesFilter.end();
                    };
                }
                machineList = browseForMachines(Umati::Dashboard::NodeId_MachinesFolder, Umati::Dashboard::NodeId_MachinesFolder, filter);
            }
            catch (const Umati::Exceptions::OpcUaException &ex) {
                LOG(ERROR) << "Browse new machines failed with: " << ex.what();
                return false;
            }
            catch (const Umati::Exceptions::ClientNotConnected &ex) {
                LOG(ERROR) << "OPC UA Client not connected." << ex.what();
                return false;
            }
            return true;
        }

        std::list<ModelOpcUa::BrowseResult_t> MachineObserver::findComponentsFolder(ModelOpcUa::NodeId_t nodeid)
        {
            std::list<ModelOpcUa::BrowseResult_t> newMachines;
            try {
                auto componentFolder = m_pDataClient->TranslateBrowsePathToNodeId(nodeid, Umati::Dashboard::QualifiedName_ComponentsFolder);
                if (!componentFolder.isNull()) {
                    newMachines = browseForMachines(componentFolder, nodeid);
                }
            }
            catch (const Umati::Exceptions::OpcUaException &ex) {}
            return newMachines;
        }

        std::list<ModelOpcUa::BrowseResult_t> MachineObserver::browseForMachines(ModelOpcUa::NodeId_t nodeid, ModelOpcUa::NodeId_t parentNodeId, std::function<bool(ModelOpcUa::NodeId_t)> filter)
        {
            std::list<ModelOpcUa::BrowseResult_t> newMachines;
            auto potentialMachines = m_pDataClient->Browse(nodeid, Dashboard::IDashboardDataClient::BrowseContext_t::Hierarchical());
            for(auto &machine: potentialMachines) {
                if(filter && !filter(machine.NodeId))
                {
                    continue;
                }
                if (machine.TypeDefinition == Dashboard::NodeId_MissingType) {
                    try {
                        auto defs = m_pDataClient->Browse(machine.NodeId,
                                                          Dashboard::IDashboardDataClient::BrowseContext_t::HasTypeDefinition());
                        machine.TypeDefinition = defs.front().NodeId;
                        LOG(INFO) << "Fixing missing type definition in opc asyncio. Parent: " << machine.NodeId
                                  << " TypeDefinition: " << machine.TypeDefinition;
                    }
                    catch (const Umati::Exceptions::UmatiException &ex) {
                        LOG(INFO) << "Could not fix missing type definition in browse result for machine " << machine.NodeId << ' ' << ex.what();
                    }
                }
                try {
                    auto subTypeToBaseType = m_pOpcUaTypeReader->m_subTypeDefinitionToKnownMachineTypeDefinition.find(machine.TypeDefinition);
                    if (subTypeToBaseType == m_pOpcUaTypeReader->m_subTypeDefinitionToKnownMachineTypeDefinition.end()) {
                        // If the machine does not have a proper, configured machine (Super)TypeDefinition, mark it as invalid
                        // to prevent search for following machines with the same invalid (Super)TypeDefinition
                        auto typeToSupertype = std::make_pair(machine.TypeDefinition, Dashboard::NodeId_UndefinedType);
                        m_pOpcUaTypeReader->m_subTypeDefinitionToKnownMachineTypeDefinition.insert(typeToSupertype);
                        for (const auto &md: m_pOpcUaTypeReader->m_knownMachineTypeDefinitions) {
                            if (m_pDataClient->isSameOrSubtype(md, machine.TypeDefinition, 3)) {
                                m_pOpcUaTypeReader->m_subTypeDefinitionToKnownMachineTypeDefinition[machine.TypeDefinition] = md;
                                machine.TypeDefinition = md;
                                break;
                            }
                        }
                    } else {
                        // Check if machines (Super)TypeDefinition is an invalid or not configured
                        if (!(subTypeToBaseType->second == Dashboard::NodeId_UndefinedType)) {
                            machine.TypeDefinition = subTypeToBaseType->second;
                        }
                    }
                    if(machine.TypeDefinition == Umati::Dashboard::NodeId_BaseObjectType) {
                        LOG(DEBUG) << "machine is a BaseObjectType : " << machine.NodeId.Uri << machine.NodeId.Id << " Search for InterfaceTypes";
                        auto ifs = m_pDataClient->Browse(machine.NodeId,
                            Dashboard::IDashboardDataClient::BrowseContext_t::HasInterface());
                        if (ifs.empty()){
                            LOG(WARNING) << "machine is a BaseObjectType without a InterfaceType :" << machine.NodeId.Uri << machine.NodeId.Id;
                        }
                        else{
                            machine.TypeDefinition = ifs.front().NodeId;
                        }
                    }

                    auto typeDefinitionNodeId = m_pOpcUaTypeReader->getIdentificationTypeNodeId(machine.TypeDefinition);
                    auto ident = m_pDataClient->BrowseWithResultTypeFilter(machine.NodeId, Dashboard::IDashboardDataClient::BrowseContext_t::Hierarchical(),
                                                                           typeDefinitionNodeId);
                    if (!ident.empty()) {
                        newMachines.push_back(machine);
                        newMachines.splice(newMachines.end(), findComponentsFolder(machine.NodeId));
                        m_parentOfMachine.insert(std::make_pair(machine.NodeId, parentNodeId));
                    } else {
                        LOG(INFO) << "Identification is empty for " << machine.NodeId.Uri << machine.NodeId.Id;
                    }
                } catch (const Umati::Exceptions::OpcUaException &ex) {
                    LOG(INFO) << "Err " << ex.what();
                } catch (const Umati::MachineObserver::Exceptions::MachineInvalidException &ex) {
                    LOG(INFO) << ex.what();
                }
            }

            return newMachines;
        }

        void MachineObserver::findNewAndOfflineMachines(std::list<ModelOpcUa::BrowseResult_t> &machineList,
                                                        std::set<ModelOpcUa::NodeId_t> &toBeRemovedMachines,
                                                        std::set<ModelOpcUa::NodeId_t> &newMachines,
                                                        std::map<ModelOpcUa::NodeId_t, nlohmann::json> &machinesIdentifications) {
            LOG(INFO) << "Checking which machines are online / offline";

            for (auto &machineTool : machineList) {

                // Check if Machine is known as online machine
                auto it = toBeRemovedMachines.find(machineTool.NodeId);

                // Machine known
                try {
                    // Check if machine is still online. If so, remove it from the removed machines. If it is not on there, it must be a new machine
                    nlohmann::json identificationAsJson;
                    if (isOnline(machineTool.NodeId, identificationAsJson, machineTool.TypeDefinition)) {
                        if (it != toBeRemovedMachines.end()) {
                            toBeRemovedMachines.erase(it);// todo or does it need to be it++?
                        } else {
                            newMachines.insert(machineTool.NodeId);
                        }
                        machinesIdentifications.insert(std::make_pair(machineTool.NodeId, identificationAsJson));
                    } else {
                        LOG(INFO) << "Machine " << machineTool.BrowseName.Name << " not identified as online";
                    }
                }
                // Catch exceptions during CheckOnline, this will cause that the machine stay in the toBeRemovedMachines list
                catch (const Umati::Exceptions::OpcUaException &) {
                    LOG(INFO) << "Machine disconnected: '" << machineTool.BrowseName.Name << "' ("
                              << machineTool.NodeId.Uri << ")";
                }
            }

            logMachinesChanging("To be removed machines: ", toBeRemovedMachines);
            logMachinesChanging("New / Staying machines: ", newMachines);
        }

        void MachineObserver::logMachinesChanging(const std::string &text,
                                                  const std::map<ModelOpcUa::NodeId_t, ModelOpcUa::BrowseResult_t> &machines) {
            std::stringstream machinesStringStream;
            for (auto &machine : machines) {
                machinesStringStream << machine.first.Uri << "\n";
            }
            LOG(INFO) << text << "\n" << machinesStringStream.str().c_str();
        }

        void MachineObserver::logMachinesChanging(const std::string &text,
                                                  const std::set<ModelOpcUa::NodeId_t> &machines) {
            std::stringstream machinesStringStream;
            for (auto &machine : machines) {
                machinesStringStream << machine.Uri << "\n";
            }
            LOG(INFO) << text << "\n" << machinesStringStream.str().c_str();
        }

        void MachineObserver::removeOfflineMachines(
                std::set<ModelOpcUa::NodeId_t> &toBeRemovedMachines) {

            logMachinesChanging("Removing machines: ", toBeRemovedMachines);

            for (auto &toBeRemovedMachine : toBeRemovedMachines) {
                removeMachine(toBeRemovedMachine);
            }
        }

        void
        MachineObserver::addNewMachine(const ModelOpcUa::BrowseResult_t &newMachine) {
            try {
                addMachine(newMachine);
                m_knownMachines.insert(std::make_pair(newMachine.NodeId, newMachine));
            }
            catch (const Exceptions::MachineInvalidException &machineInvalidException) {
                LOG(INFO) << "Machine invalid: " << static_cast<std::string>(newMachine.NodeId);
                m_invalidMachines.insert(std::make_pair(newMachine.NodeId, std::make_pair(NumSkipAfterInvalid, machineInvalidException.what())));
            }
            catch (const Exceptions::MachineOfflineException &machineOfflineException) {
                LOG(INFO) << "Machine offline: " << static_cast<std::string>(newMachine.NodeId);
                m_invalidMachines.insert(std::make_pair(newMachine.NodeId, std::make_pair(NumSkipAfterInvalid, machineOfflineException.what())));
            }
        }
    }
}
