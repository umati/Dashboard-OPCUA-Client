#include "DashboardMachineObserver.hpp"
#include <easylogging++.h>
#include "Exceptions/MachineInvalidException.hpp"
#include <Exceptions/OpcUaException.hpp>
#include <TypeDefinition/UmatiTypeNodeIds.hpp>

namespace Umati {
    namespace MachineObserver {

        DashboardMachineObserver::DashboardMachineObserver(
                std::shared_ptr<Dashboard::IDashboardDataClient> pDataClient,
                std::shared_ptr<Umati::Dashboard::IPublisher> pPublisher
        ) : MachineObserver(pDataClient),
            m_pPublisher(pPublisher) {
            startUpdateMachineThread();
        }

        DashboardMachineObserver::~DashboardMachineObserver() {
            stopMachineUpdateThread();
        }

        void DashboardMachineObserver::PublishAll() {
            std::unique_lock<decltype(m_dashboardClients_mutex)> ul(m_dashboardClients_mutex);
            for (const auto &pDashClient : m_dashboardClients) {
                pDashClient.second->Publish();
            }

            m_publishMachinesOnline = m_publishMachinesOnline + 1;
            if (m_publishMachinesOnline % 5 == 0) {
                this->publishMachinesList();
            }
        }

        void DashboardMachineObserver::startUpdateMachineThread() {
            if (m_running) {
                LOG(INFO) << "Machine update thread already running";
                return;
            }

            auto func = [this]() {
                int cnt = 0;
                while (this->m_running) {
                    if ((cnt % 10) == 0) {
                        this->UpdateMachines();
                    }

                    ++cnt;
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }
            };
            m_running = true;
            m_updateMachineThread = std::thread(func);
        }

        void DashboardMachineObserver::stopMachineUpdateThread() {
            m_running = false;
            if (m_updateMachineThread.joinable()) {
                m_updateMachineThread.join();
            }
        }

        void DashboardMachineObserver::publishMachinesList() {
            std::map<std::string, nlohmann::json> publishData;
            for (auto &machineOnline : m_onlineMachines) {

                nlohmann::json identificationAsJson;
                isOnline(machineOnline.first, identificationAsJson);

                std::string fair = machineOnline.second.Fair;
                auto findFairListIterator = publishData.find(fair);
                if (findFairListIterator == publishData.end()) {
                    publishData.insert(std::make_pair(fair, nlohmann::json::array()));
                }

                auto fairList = publishData.find(fair);

                if (!identificationAsJson.empty()) {
                    fairList->second.push_back(identificationAsJson);
                }
            }

            for (const auto &fairList : publishData) {
                std::stringstream stream;
                stream << "/umati/" << fairList.first << "/machineList";
                m_pPublisher->Publish(stream.str(), fairList.second.dump(0));
            }
        }

        void DashboardMachineObserver::addMachine(ModelOpcUa::BrowseResult_t machine) {
            try {
                LOG(INFO) << "New Machine: " << machine.BrowseName.Name << " NodeId:"
                          << static_cast<std::string>(machine.NodeId);

                auto pDashClient = std::make_shared<Umati::Dashboard::DashboardClient>(m_pDataClient, m_pPublisher);

                MachineInformation_t machineInformation;
                machineInformation.NamespaceURI = machine.NodeId.Uri;
                machineInformation.StartNodeId = machine.NodeId;
                machineInformation.MachineName = machine.BrowseName.Name;
                machineInformation.Fair = "offsite";

                std::shared_ptr<ModelOpcUa::StructureNode> p_type = getTypeOfNamespace(machine.NodeId);

                std::stringstream topic;
                topic << "/umati/" << machineInformation.Fair << "/" << machineInformation.MachineName;
                pDashClient->addDataSet(
                        {machineInformation.NamespaceURI, machine.NodeId.Id},
                        p_type,
                        topic.str()
                );

                LOG(INFO) << "Read model finished";

                {
                    std::unique_lock<decltype(m_dashboardClients_mutex)> ul(m_dashboardClients_mutex);
                    m_dashboardClients.insert(std::make_pair(machine.NodeId, pDashClient));
                    m_onlineMachines.insert(std::make_pair(machine.NodeId, machineInformation));
                    m_machineNames.insert(std::make_pair(machine.NodeId, machine.BrowseName.Name));
                }
            }
            catch (const Umati::Exceptions::OpcUaException &ex) {
                LOG(ERROR) << "Could not add Machine " << machine.BrowseName.Name
                           << " NodeId:" << static_cast<std::string>(machine.NodeId) <<
                           "OpcUa Error: " << ex.what();

                throw Exceptions::MachineInvalidException(static_cast<std::string>(machine.NodeId));
            }
        }

        std::shared_ptr<ModelOpcUa::StructureNode> DashboardMachineObserver::getTypeOfNamespace(ModelOpcUa::NodeId_t nodeId) const {
            std::shared_ptr<ModelOpcUa::StructureNode> p_type;

            uint machineTypeNamespaceIndex = getImplementedNamespaceIndex(nodeId);

            std::string typeName = m_pDataClient->m_availableObjectTypeNamespaces[machineTypeNamespaceIndex].NamespaceType;
            auto typePair = m_pDataClient->m_typeMap->find(typeName);
            ModelOpcUa::StructureNode type = typePair->second;
            p_type = std::make_shared<ModelOpcUa::StructureNode>(type);
            return p_type;
        }


        std::shared_ptr<ModelOpcUa::StructureNode> DashboardMachineObserver::getIdentificationTypeOfNamespace(ModelOpcUa::NodeId_t nodeId) const {
            std::shared_ptr<ModelOpcUa::StructureNode> p_type;

            uint machineTypeNamespaceIndex = getImplementedNamespaceIndex(nodeId);

            std::string identificationTypeName = m_pDataClient->m_availableObjectTypeNamespaces[machineTypeNamespaceIndex].NamespaceIdentificationType;
            auto typePair = m_pDataClient->m_typeMap->find(identificationTypeName);
            ModelOpcUa::StructureNode type = typePair->second;
            p_type = std::make_shared<ModelOpcUa::StructureNode>(type);
            return p_type;
        }

        uint DashboardMachineObserver::getImplementedNamespaceIndex(const ModelOpcUa::NodeId_t &nodeId) const {
            UaReferenceDescriptions machineTypeDefinitionReferenceDescriptions;
            auto startFromMachineNodeId = UaNodeId::fromXmlString(UaString(nodeId.Id.c_str()));
            uint machineNamespaceIndex = m_pDataClient->m_uriToIndexCache[nodeId.Uri];
            startFromMachineNodeId.setNamespaceIndex(machineNamespaceIndex);

            UaClientSdk::BrowseContext browseContext;
            browseContext.referenceTypeId = OpcUaId_HasTypeDefinition;
            browseContext.browseDirection = OpcUa_BrowseDirection_Forward;
            browseContext.includeSubtype = OpcUa_True;
            browseContext.maxReferencesToReturn = 0;
            browseContext.nodeClassMask = 0; // ALL
            browseContext.resultMask = OpcUa_BrowseResultMask_All;
            m_pDataClient->browseUnderStartNode(startFromMachineNodeId, machineTypeDefinitionReferenceDescriptions, browseContext);

            uint machineTypeNamespaceIndex = 0;
            for (OpcUa_UInt32 i = 0; i < machineTypeDefinitionReferenceDescriptions.length(); i++) {
                machineTypeNamespaceIndex = machineTypeDefinitionReferenceDescriptions[i].NodeId.NodeId.NamespaceIndex;
            }
            return machineTypeNamespaceIndex;
        }

        void DashboardMachineObserver::removeMachine(ModelOpcUa::BrowseResult_t machine) {
            std::unique_lock<decltype(m_dashboardClients_mutex)> ul(m_dashboardClients_mutex);
            LOG(INFO) << "Remove Machine: " << machine.BrowseName.Name << " NodeId:"
                      << static_cast<std::string>(machine.NodeId);
            auto it = m_dashboardClients.find(machine.NodeId);
            if (it != m_dashboardClients.end()) {
                m_dashboardClients.erase(it); // todo or does it need to be it++ ?
            } else {
                LOG(INFO) << "Machine not known: '" << static_cast<std::string>(machine.NodeId) << "'";
            }

            auto itOnlineMachines = m_onlineMachines.find(machine.NodeId);
            if (itOnlineMachines != m_onlineMachines.end()) {
                LOG(INFO) << "Erasing online machine";
                m_onlineMachines.erase(itOnlineMachines); // todo or dies it need to be itOnlineMachines++?
                LOG(INFO) << "Online machine erased";
            } else {
                LOG(INFO) << "Machine was not online: '" << static_cast<std::string>(machine.NodeId) << "'";
            }
        }

        bool
        DashboardMachineObserver::isOnline(const ModelOpcUa::NodeId_t &machineNodeId, nlohmann::json &identificationAsJson) {

            std::shared_ptr<ModelOpcUa::StructureNode> p_type = getIdentificationTypeOfNamespace(machineNodeId);
            ModelOpcUa::NodeId_t type = Dashboard::TypeDefinition::NodeIds::MachineToolIdentificationType;// todo ! change
            std::string typeName = machineNodeId.Uri;

            std::stringstream identificationTypeName;
            identificationTypeName << "MachineTool" << "IdentificationType";
            auto typePair = m_pDataClient->m_typeMap->find(identificationTypeName.str());


            auto hasComponents = ModelOpcUa::NodeId_t{"",std::to_string(OpcUaId_HasComponent)};
            //auto childNodeId = m_pDataClient->TranslateBrowsePathToNodeId(startNode, pChild->SpecifiedBrowseName);

            std::list<ModelOpcUa::BrowseResult_t> identification = m_pDataClient->Browse(machineNodeId, hasComponents, type);
            if (!identification.empty()) {
                UaReferenceDescriptions referenceDescriptions;
                browseIdentificationValues(machineNodeId,identification, referenceDescriptions, identificationAsJson);
                if (!identificationAsJson.empty()) {
                    return true;
                }
            }

            return false;
        }

        void DashboardMachineObserver::browseIdentificationValues(ModelOpcUa::NodeId_t machineNodeId,
                                                                    std::list<ModelOpcUa::BrowseResult_t> &identification,
                                                                  UaReferenceDescriptions &referenceDescriptions,
                                                                  nlohmann::json &identificationAsJson) const {
            std::vector<nlohmann::json> identificationListValues;
            auto startNodeId = UaNodeId::fromXmlString(UaString(identification.front().NodeId.Id.c_str()));
            startNodeId.setNamespaceIndex(m_pDataClient->m_uriToIndexCache[identification.front().NodeId.Uri]);
            m_pDataClient->browseUnderStartNode(startNodeId, referenceDescriptions);


            std::list<ModelOpcUa::NodeId_t> identificationNodes;
            std::vector<std::string> identificationValueKeys;
            for (OpcUa_UInt32 i = 0; i < referenceDescriptions.length(); i++) {
                ModelOpcUa::BrowseResult_t browseResult = m_pDataClient->ReferenceDescriptionToBrowseResult(referenceDescriptions[i]);
                identificationValueKeys.push_back(browseResult.BrowseName.Name);
                identificationNodes.emplace_back(browseResult.NodeId);
            }

            identificationListValues = m_pDataClient->readValues(identificationNodes);
            for(uint i = 0; i < identificationListValues.size(); i++) {
                auto value = identificationListValues.at(i);
                if(value.dump(0) != "null") {
                    identificationAsJson[identificationValueKeys.at(i)] = value;
                }
            }

            std::stringstream path;
            auto it = m_machineNames.find(machineNodeId);
            if (it != m_machineNames.end()) {
                path << "/offsite/" << it->second;
                identificationAsJson["Path"] = path.str();
            }
        }
    }
}
