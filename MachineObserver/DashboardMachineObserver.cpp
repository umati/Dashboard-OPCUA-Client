#include "DashboardMachineObserver.hpp"

#include <easylogging++.h>



#include "MachineCacheJsonFile.hpp"

#include "Exceptions/MachineInvalidException.hpp"
#include "Exceptions/NoPublishTopicSet.hpp"

#include <Exceptions/OpcUaException.hpp>
#include <TypeDefinition/UmatiTypeDefinition.hpp>

namespace Umati {
	namespace MachineObserver {

		DashboardMachineObserver::DashboardMachineObserver(
			std::shared_ptr<Dashboard::IDashboardDataClient> pDataClient,
			std::shared_ptr<Umati::Dashboard::IPublisher> pPublisher,
			std::string machineCacheFilename
		) : MachineObserver(pDataClient),
			m_pPublisher(pPublisher),
			m_pubTopicFactory(std::make_shared<MachineCacheJsonFile>(machineCacheFilename))
		{
			startUpdateMachineThread();
		}

		DashboardMachineObserver::~DashboardMachineObserver()
		{
			stopMachineUpdateThread();
		}

		void DashboardMachineObserver::PublishAll()
		{

			std::unique_lock<decltype(m_dashboardClients_mutex)> ul(m_dashboardClients_mutex);
			for (auto pDashClient : m_dashboardClients)
			{
				pDashClient.second->Publish();
			}

			if ((--m_publishMachinesOnline) <= 0)
			{
				m_publishMachinesOnline = PublishMachinesListResetValue;

				this->publishMachinesList();
			}
		}

		void DashboardMachineObserver::startUpdateMachineThread()
		{
			if (m_running)
			{
				LOG(INFO) << "Machine update thread already running";
				return;
			}

			auto func = [this]()
			{
				int cnt = 0;
				while (this->m_running)
				{
					if ((cnt % 10) == 0)
					{
						this->UpdateMachines();
					}

					++cnt;
					std::this_thread::sleep_for(std::chrono::seconds(1));
				}
			};
			m_running = true;

			m_updateMachineThread = std::thread(func);
		}

		void DashboardMachineObserver::stopMachineUpdateThread()
		{
			m_running = false;
			if (m_updateMachineThread.joinable())
			{
				m_updateMachineThread.join();
			}
		}

		void DashboardMachineObserver::publishMachinesList()
		{
			std::map<std::string, nlohmann::json> publishData;
			for (auto &machineOnline : m_onlineMachines)
			{

				std::string fair = "offsite";
                std::string manufacturer;
                std::string machine_name;
                std::string heyJson;
                std::vector<ModelOpcUa::QualifiedName_t> identifierIds;

                /*
                for (auto it = type.SpecifiedChildNodes.begin(); it != type.SpecifiedChildNodes.end(); it++) {
                    identifierIds.emplace_back(it->get()->SpecifiedBrowseName);
                }
                */
                ModelOpcUa::NodeId_t type = Dashboard::TypeDefinition::NodeIds::MachineToolIdentificationType;// todo ! change
                int namespaceIndex = 5;// todo ! change
                std::list<ModelOpcUa::BrowseResult_t> identification = m_pDataClient->Browse(machineOnline.first,Dashboard::TypeDefinition::HasComponent,type);
                if(!identification.empty()) {

                    UaReferenceDescriptions referenceDescriptions;
                    std::vector<nlohmann::json> identificationListValues;
                    browseIdentificationValues(identification, namespaceIndex, referenceDescriptions, identificationListValues);
                    if(!identificationListValues.empty()) {
                        heyJson = identificationValuesToJsonString(referenceDescriptions, identificationListValues, fair,manufacturer,machine_name);
                    }
                }

				auto findFairListIterator = publishData.find(fair);
				if (findFairListIterator == publishData.end())
				{
					publishData.insert(std::make_pair(fair, nlohmann::json::array()));
				}
				auto fairList = publishData.find(fair);

                if(!heyJson.empty()) {
                    fairList->second.push_back(heyJson);
                }
			}

			for (const auto& fairList : publishData) {
				std::stringstream stream;
				stream << MachinesListTopic1 << fairList.first << MachinesListTopic2;
				m_pPublisher->Publish(stream.str(), fairList.second.dump(2));
			}
		}

		/**
		* looks for the node 5005 (list of stacklights) 5024 (Tools), 5007 (StateMOde) and more
		*/
		void DashboardMachineObserver::addMachine(ModelOpcUa::BrowseResult_t machine)
		{
			try {
				LOG(INFO) << "New Machine: " << machine.BrowseName.Name << " NodeId:" << static_cast<std::string>(machine.NodeId);

				auto pDashClient = std::make_shared<Umati::Dashboard::DashboardClient>(m_pDataClient, m_pPublisher);
				auto pubTopics = m_pubTopicFactory.getPubTopics(machine);
				auto nsUri = machine.NodeId.Uri;

				LOG(INFO) << "Prefix '" << pubTopics.TopicPrefix << "' for machine " << machine.NodeId.Uri;

				if (!pubTopics.isValid())
				{
					std::stringstream ss;
					ss << "Invalid publisher topic. Machine: " << machine.BrowseName.Name
						<< " NodeId:" << static_cast<std::string>(machine.NodeId);
					LOG(ERROR) << ss.str();
					throw Exceptions::NoPublishTopicSet(ss.str());
				}

				MachineInformation_t machineInformation;
				machineInformation.TopicPrefix = pubTopics.TopicPrefix;
				machineInformation.NamespaceURI = machine.NodeId.Uri;
				machineInformation.StartNodeId = machine.NodeId;

                UaReferenceDescriptions machineComponentsReferenceDescriptions;
                UaReferenceDescriptions singleComponentReferenceDescriptions;
                auto startFromMachineNodeId = UaNodeId::fromXmlString(UaString(machine.NodeId.Id.c_str()));
                uint namespaceIdx = m_pDataClient->m_uriToIndexCache[machine.NodeId.Uri];
                startFromMachineNodeId.setNamespaceIndex(namespaceIdx);


                UaClientSdk::BrowseContext browseContext;
                browseContext.referenceTypeId = OpcUaId_HierarchicalReferences;
                browseContext.browseDirection = OpcUa_BrowseDirection_Forward;
                browseContext.includeSubtype = OpcUa_True;
                browseContext.maxReferencesToReturn = 0;
                browseContext.nodeClassMask = 0; // ALL
                browseContext.resultMask = OpcUa_BrowseResultMask_All;

                //m_pDataClient->browseUnderStartNode(startNodeId, referenceDescriptions);
                m_pDataClient->browseUnderStartNode(startFromMachineNodeId, machineComponentsReferenceDescriptions, browseContext);


                std::shared_ptr<ModelOpcUa::StructureNode> p_type;
                for (OpcUa_UInt32 machineComponent = 0; machineComponent < machineComponentsReferenceDescriptions.length(); machineComponent++) {
                    ModelOpcUa::BrowseResult_t machineComponentBrowseResult = m_pDataClient->ReferenceDescriptionToBrowseResult(machineComponentsReferenceDescriptions[machineComponent]);

                    LOG(INFO) << machineComponentBrowseResult.BrowseName.Name;

                    browseContext.referenceTypeId = OpcUaId_NonHierarchicalReferences;

                    auto startFromMachineComponentNodeId = UaNodeId::fromXmlString(UaString(machineComponentBrowseResult.NodeId.Id.c_str()));
                    startFromMachineComponentNodeId.setNamespaceIndex(namespaceIdx);

                    m_pDataClient->browseUnderStartNode(startFromMachineComponentNodeId, singleComponentReferenceDescriptions, browseContext);

                    for (OpcUa_UInt32 j = 0; j < singleComponentReferenceDescriptions.length(); j++) {

                        ModelOpcUa::BrowseResult_t browseResult2 = m_pDataClient->ReferenceDescriptionToBrowseResult(singleComponentReferenceDescriptions[j]);

                        ModelOpcUa::StructureNode type = m_pDataClient->m_typeMap->find(browseResult2.BrowseName.Name)->second;
                        p_type = std::make_shared<ModelOpcUa::StructureNode>(type);

                        // todo browse nodes and check if required children are there
                        uint referenceTypeId = OpcUaId_HasProperty;
                        UaReferenceDescriptions referenceDescriptions3;

                        std::list<ModelOpcUa::BrowseResult_t> result2;
                        auto nextnextNodeId = UaNodeId::fromXmlString(UaString(machineComponentBrowseResult.NodeId.Id.c_str()));
                        nextnextNodeId.setNamespaceIndex(namespaceIdx);
                        browseContext.referenceTypeId = referenceTypeId;
                        m_pDataClient->browseUnderStartNode(nextnextNodeId, referenceDescriptions3, browseContext);
                        for(uint z = 0; z < referenceDescriptions3.length(); z++) {
                            ModelOpcUa::BrowseResult_t entry = m_pDataClient->ReferenceDescriptionToBrowseResult(referenceDescriptions3[z]);
                            result2.push_back(entry);
                            bool foundChild = false;
                            for(auto childIt = type.SpecifiedChildNodes.begin(); childIt != type.SpecifiedChildNodes.end(); childIt++) {
                                if(entry.BrowseName.Name == childIt.operator*()->SpecifiedBrowseName.Name) {
                                    foundChild=true;
                                    break;
                                }
                            }
                            LOG(INFO) << entry.BrowseName.Name << (foundChild ? " found" : " not found");
                        }




                        pDashClient->addDataSet(
                                {nsUri, machineComponentBrowseResult.NodeId.Id},
                                p_type,
                                pubTopics.Information
                        );
                    }

                }

				LOG(INFO) << "Read model finished";


				{
					std::unique_lock<decltype(m_dashboardClients_mutex)> ul(m_dashboardClients_mutex);
					m_dashboardClients.insert(std::make_pair(machine.NodeId, pDashClient));
					m_onlineMachines.insert(std::make_pair(machine.NodeId, machineInformation));
				}

			}
			catch (const Umati::Exceptions::OpcUaException &ex)
			{
				LOG(ERROR) << "Could not add Machine " << machine.BrowseName.Name
					<< " NodeId:" << static_cast<std::string>(machine.NodeId) <<
					"OpcUa Error: " << ex.what();

				throw Exceptions::MachineInvalidException(static_cast<std::string>(machine.NodeId));
			}
		}

		void DashboardMachineObserver::removeMachine(
			ModelOpcUa::BrowseResult_t machine
		)
		{
			std::unique_lock<decltype(m_dashboardClients_mutex)> ul(m_dashboardClients_mutex);
			LOG(INFO) << "Remove Machine: " << machine.BrowseName.Name << " NodeId:" << static_cast<std::string>(machine.NodeId);
			auto it = m_dashboardClients.find(machine.NodeId);
			if (it != m_dashboardClients.end())
			{
				m_dashboardClients.erase(it); // todo or does it need to be it++ ?
			}
			else
			{
				LOG(INFO) << "Machine not known: '" << static_cast<std::string>(machine.NodeId) << "'";
			}

			auto itOnlineMachines = m_onlineMachines.find(machine.NodeId);
			if (itOnlineMachines != m_onlineMachines.end())
			{
			    LOG(INFO) << "Erasing online machine";
				m_onlineMachines.erase(itOnlineMachines); // todo or dies it need to be itOnlineMachines++?
                LOG(INFO) << "Online machine erased";
            }
			else
			{
				LOG(INFO) << "Machine was not online: '" << static_cast<std::string>(machine.NodeId) << "'";
			}
		}

		/*
		* searches for the node id i=6001 in the namespace of the machine
		**/
		bool DashboardMachineObserver::isOnline(ModelOpcUa::BrowseResult_t machine)
		{
		    // todo get the identification type here and browse all of its elements
            // auto type = m_pDataClient->m_typeMap->find("MachineToolIdentificationType")->second;
            ModelOpcUa::NodeId_t type = Dashboard::TypeDefinition::NodeIds::MachineToolIdentificationType;
            std::vector<ModelOpcUa::QualifiedName_t> identifierIds;

            /*
            for (auto it = type.SpecifiedChildNodes.begin(); it != type.SpecifiedChildNodes.end(); it++) {
                identifierIds.emplace_back(it->get()->SpecifiedBrowseName);
            }
            */
            std::list<ModelOpcUa::BrowseResult_t> identification = m_pDataClient->Browse(machine.NodeId,Dashboard::TypeDefinition::HasComponent,type);
            int namespaceIndex = 5;// todo change
            if(!identification.empty()) {

                UaReferenceDescriptions referenceDescriptions;
                std::vector<nlohmann::json> identificationListValues;
                browseIdentificationValues(identification, namespaceIndex, referenceDescriptions, identificationListValues);
                if(identificationListValues.size() > 0) {
                    std::string fair;
                    std::string manufacturer;
                    std::string machine_name;
                    std::string heyJson = identificationValuesToJsonString(referenceDescriptions,
                                                                           identificationListValues, fair, manufacturer,
                                                                           machine_name);


                    std::string topic = getMachineIsOnlineTopic(fair, manufacturer, machine_name);
                    bool online = true;
                    std::string payload = nlohmann::json(online).dump(2);
                    m_pPublisher->Publish(topic, payload);
                    m_pPublisher->Publish(topic, heyJson);

                return true;
                }
            }
			return false;
		}

        std::string
        DashboardMachineObserver::getMachineIsOnlineTopic(const std::string &fair, std::string &manufacturer,
                                                          std::string &machine_name) const {
            std::replace(manufacturer.begin(), manufacturer.end(), ' ', '_');
            std::replace( machine_name.begin(), machine_name.end(), ' ', '_');
            std::stringstream onlineTopicStream;
            onlineTopicStream << "/umati/" << fair << '/' << manufacturer << '/' << machine_name;
            std::string topic = onlineTopicStream.str();
            return topic;
        }

        std::string
        DashboardMachineObserver::identificationValuesToJsonString(const UaReferenceDescriptions &referenceDescriptions,
                                                                   const std::vector<nlohmann::json> &identificationListValues,
                                                                   std::string &fair, std::string &manufacturer,
                                                                   std::string &machine_name) const {
            fair= "offsite";
            std::stringstream jsonstream;
            jsonstream << '{';
            for (OpcUa_UInt32 i = 0; i < referenceDescriptions.length(); i++) {
                ModelOpcUa::BrowseResult_t browseResult = m_pDataClient->ReferenceDescriptionToBrowseResult(referenceDescriptions[i]);
                std::string s;
                if (identificationListValues.at(i)["value"].is_string()) {
                    s =  identificationListValues.at(i)["value"].get<std::string>();
                }
                if (identificationListValues.at(i)["value"].is_structured()){
                    s =  identificationListValues.at(i)["value"]["text"].get<std::string>();
                }
                if (!s.empty()) {
                    LOG(INFO) << browseResult.BrowseName.Name << ", " << s;
                    jsonstream << '"' << browseResult.BrowseName.Name << '"'<<':'<<'"' << s << '"' << ','  ;
                }
                if(browseResult.BrowseName.Name == "Manufacturer") {
                    manufacturer = s;
                }
                if(browseResult.BrowseName.Name == "Model") {
                    machine_name = s;
                }
            }
            jsonstream.seekp(-1,  std::ios_base::end);
            jsonstream << "}";
            return jsonstream.str();
        }

        void DashboardMachineObserver::browseIdentificationValues(std::list<ModelOpcUa::BrowseResult_t> &identification,
                                                                  int namespaceIndex,
                                                                  UaReferenceDescriptions &referenceDescriptions,
                                                                  std::vector<nlohmann::json> &identificationListValues) const {

            auto startNodeId = UaNodeId::fromXmlString(UaString(identification.front().NodeId.Id.c_str()));
            startNodeId.setNamespaceIndex(namespaceIndex);
            m_pDataClient->browseUnderStartNode(startNodeId, referenceDescriptions);

            std::list<ModelOpcUa::NodeId_t> identificationNodes;
            for (OpcUa_UInt32 i = 0; i < referenceDescriptions.length(); i++) {
                ModelOpcUa::BrowseResult_t browseResult = m_pDataClient->ReferenceDescriptionToBrowseResult(referenceDescriptions[i]);
                identificationNodes.emplace_back(browseResult.NodeId);
            }

            identificationListValues= m_pDataClient->readValues(identificationNodes);
        }

// todo: getMachineToolIdentification ==> Read "MachineToolIdentificationType" of the machine
// todo update this to the new impl, let it read the name and type and return a json
		void DashboardMachineObserver::updateMachinesMachineData(
			DashboardMachineObserver::MachineInformation_t &machineInformation
		)
		{
			const std::string NodeIdIdentifier_NameCustom("i=6003");
			const std::string NodeIdIdentifier_LocationMachine("i=6004");
			const std::string NodeIdIdentifier_LocationPlant("i=6005");
			const std::string NodeIdIdentifier_Manufacturer("i=6006");
			auto valuesList = m_pDataClient->readValues(
				{
					{machineInformation.NamespaceURI, NodeIdIdentifier_LocationPlant},
					{machineInformation.NamespaceURI, NodeIdIdentifier_Manufacturer},
					{machineInformation.NamespaceURI, NodeIdIdentifier_NameCustom},
					{machineInformation.NamespaceURI, NodeIdIdentifier_LocationMachine},
				}
			);

			machineInformation.LocationPlant = getValueFromValuesList(valuesList, "LocationPlant", 0, machineInformation.StartNodeId);
			machineInformation.DisplayManufacturer = getValueFromValuesList(valuesList, "DisplayManufacturer", 1, machineInformation.StartNodeId);
			machineInformation.DisplayName = getValueFromValuesList(valuesList, "DisplayName", 2, machineInformation.StartNodeId);
			machineInformation.LocationMachine = getValueFromValuesList(valuesList, "LocationMachine", 3, machineInformation.StartNodeId);
		}

		std::string DashboardMachineObserver::getValueFromValuesList(std::vector<nlohmann::json, std::allocator<nlohmann::json>>& valuesList, std::string valueName, int valueIndex, ModelOpcUa::NodeId_t startNodeId)
		{
			if (!valuesList.at(valueIndex)["value"].is_string())
			{
				std::stringstream ss;
				ss << valueName << " is not a string.Machine-NodeId: " << static_cast<std::string>(startNodeId);
				LOG(ERROR) << ss.str();
				throw Exceptions::MachineInvalidException(ss.str());
			}
			//LOG(INFO) << valueName << ": " << valuesList.at(valueIndex)["value"].get<std::string>();
			return valuesList.at(valueIndex)["value"].get<std::string>();
		}
	}
}
