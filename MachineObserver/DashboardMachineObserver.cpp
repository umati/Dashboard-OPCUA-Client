 /* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 * 
 * Copyright 2019-2021 (c) Christian von Arnim, ISW University of Stuttgart (for umati and VDW e.V.)
 * Copyright 2020 (c) Dominik Basner, Sotec GmbH (for VDW e.V.)
 * Copyright 2021 (c) Moritz Walker, ISW University of Stuttgart (for umati and VDW e.V.)
 * Copyright 2021 (c) Marius Dege, basysKom GmbH
 * Copyright 2021 (c) Frank Meerkoetter, basysKom GmbH
 */

#include "DashboardMachineObserver.hpp"
#include <easylogging++.h>
#include "Exceptions/MachineInvalidException.hpp"
#include <Exceptions/OpcUaException.hpp>
#include <utility>
#include <Topics.hpp>
#include <IdEncode.hpp>
#include "PublishMachinesList.hpp"
#include <open62541/client_subscriptions.h>

namespace Umati
{
	namespace MachineObserver
	{
		static DashboardMachineObserver* dbmo;
		static void handler_events(UA_Client *client, UA_UInt32 subId, void *subContext, UA_UInt32 monId, void *monContext, size_t nEventFields, UA_Variant *eventFields) {
    		//LOG(INFO) << "ModelChangeEvent";
			UA_NodeId* sourceNodeId;
			UA_NodeId affectedNode;
			UA_NodeId affectedType;
			UA_Byte verb;
			for(size_t i = 0; i < nEventFields; ++i) {
				UA_Variant eventField = eventFields[i];
        		if(UA_Variant_hasScalarType(&eventField, &UA_TYPES[UA_TYPES_NODEID])) {
					UA_NodeId* nodeId = (UA_NodeId *)eventField.data;
					//LOG(INFO) << "NodeId: " << nodeId->identifier.numeric;
					if(i == 1) {
						sourceNodeId = nodeId;
					}
				}  else {
					UA_ExtensionObject* extensionObjects = (UA_ExtensionObject*)eventField.data;
					UA_ModelChangeStructureDataType modelChangeStructureDataTypes[eventField.arrayLength];
					for(int j = 0; j < eventField.arrayLength; j ++) {
						UA_ExtensionObject extensionObject = extensionObjects[j];
						UA_ModelChangeStructureDataType* modelChangeStructureDataType =(UA_ModelChangeStructureDataType*)extensionObject.content.decoded.data;
						modelChangeStructureDataTypes[j] = *modelChangeStructureDataType;
					}

					dbmo -> updateAfterModelChangeEvent(modelChangeStructureDataTypes, eventField.arrayLength);
					/*for(int j = 0; j < eventField.arrayLength; j ++) {
						UA_ExtensionObject extensionObject = extensionObjects[j];
						UA_ModelChangeStructureDataType* modelChangeStructureDataType =(UA_ModelChangeStructureDataType*)extensionObject.content.decoded.data;
						affectedNode = modelChangeStructureDataType->affected;
						affectedType = modelChangeStructureDataType->affectedType;
						verb = modelChangeStructureDataType->verb;
						LOG(INFO) << "SourceNodeId: " << sourceNodeId->identifier.numeric;
						LOG(INFO) << "affectedNode: " << affectedNode.identifier.numeric;
						LOG(INFO) << "affectedType: " << affectedType.identifier.numeric;
						LOG(INFO) << "verb: " << verb;
						dbmo -> updateProductionPlan(, verb)
					}*/
				}
			}
			

		}


		DashboardMachineObserver::DashboardMachineObserver(
			std::shared_ptr<Dashboard::IDashboardDataClient> pDataClient,
			std::shared_ptr<Umati::Dashboard::IPublisher> pPublisher,
			std::shared_ptr<Umati::Dashboard::OpcUaTypeReader> pOpcUaTypeReader) : MachineObserver(std::move(pDataClient), std::move(pOpcUaTypeReader)),
																				   m_pPublisher(std::move(pPublisher))
		{
			std::shared_ptr<UA_Client> client = m_pDataClient->getUaClient();
			UA_Variant value;
			UA_Variant_init(&value);
			const UA_NodeId nodeIdNameSpaceArray = UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_NAMESPACEARRAY);
			UA_Client_readValueAttribute(client.get(), nodeIdNameSpaceArray, &value);
			UA_String* strValues = (UA_String*)value.data;
			for(unsigned int i = 0; i < value.arrayLength; i++) {
				UA_String strValue = strValues[i];
				char* convert = (char*)UA_malloc(sizeof(char)*(strValue.length+1));
				memcpy(convert, strValue.data, strValue.length );
				convert[strValue.length] = '\0';
				namespaces[i] = std::string(convert);
			}
			AddSubscription();
			startUpdateMachineThread();
		}

		DashboardMachineObserver::~DashboardMachineObserver()
		{
			stopMachineUpdateThread();
		}
		
		void DashboardMachineObserver::updateAfterModelChangeEvent(UA_ModelChangeStructureDataType* modelChangeStructureDataTypes, size_t nModelChangeStructureDataTypes)
		{
			bool nodeAdded = false;
			bool nodeDeleted = false;
			bool referenceAdded = false;
			bool referenceDeleted = false;
			bool dataTypeChanged = false;
			std::list<std::shared_ptr<Umati::Dashboard::DashboardClient>> changedDbcs = std::list<std::shared_ptr<Umati::Dashboard::DashboardClient>>();
			for(int i = 0; i < nModelChangeStructureDataTypes; i++) {
				UA_ModelChangeStructureDataType modelChangeStructureDataType = modelChangeStructureDataTypes[i];
				UA_NodeId affectedNode = modelChangeStructureDataType.affected;
				UA_NodeId affectedType = modelChangeStructureDataType.affectedType;
				UA_Byte verb = modelChangeStructureDataType.verb;
				/*LOG(INFO) << "affectedNode: " << affectedNode.identifier.numeric;
				LOG(INFO) << "affectedType: " << affectedType.identifier.numeric;
				LOG(INFO) << "verb: " << verb;*/

				if((verb & UA_MODELCHANGESTRUCTUREVERBMASK_NODEADDED) == UA_MODELCHANGESTRUCTUREVERBMASK_NODEADDED) {nodeAdded = true;}
				if((verb & UA_MODELCHANGESTRUCTUREVERBMASK_NODEDELETED) == UA_MODELCHANGESTRUCTUREVERBMASK_NODEDELETED) {nodeDeleted = true;}
				if((verb & UA_MODELCHANGESTRUCTUREVERBMASK_REFERENCEADDED) == UA_MODELCHANGESTRUCTUREVERBMASK_REFERENCEADDED) {referenceAdded = true;}
				if((verb & UA_MODELCHANGESTRUCTUREVERBMASK_REFERENCEDELETED) == UA_MODELCHANGESTRUCTUREVERBMASK_REFERENCEDELETED) {referenceDeleted = true;}
				if((verb & UA_MODELCHANGESTRUCTUREVERBMASK_DATATYPECHANGED) == UA_MODELCHANGESTRUCTUREVERBMASK_DATATYPECHANGED) {dataTypeChanged = true;}
				/* Find the Dashboardclient for the event. Check uris with namespaces.*/
				ModelOpcUa::NodeId_t typeId = ModelOpcUa::NodeId_t();
				typeId.Id = "i=" + std::to_string(affectedType.identifier.numeric);
				

				affectedType.namespaceIndex;
				typeId.Uri = namespaces[affectedType.namespaceIndex];
				//typeId.Uri = "http://opcfoundation.org/UA/Glass/Flat/";
				ModelOpcUa::NodeId_t nodeId = ModelOpcUa::NodeId_t();
				nodeId.Id = "i=" + std::to_string(affectedNode.identifier.numeric);
				
				for(auto it = m_dashboardClients.begin(); it != m_dashboardClients.end(); it++) {
					std::shared_ptr<Umati::Dashboard::DashboardClient> dbc = it -> second;
					nodeId.Uri = (it -> first).Uri;
					//typeId.Uri = (it -> first).Uri;
					if(dbc->containsNodeId(nodeId)) {
						changedDbcs.push_back(dbc);
						if(nodeAdded || referenceAdded) {dbc->updateAddDataSet(nodeId);}
						if(nodeDeleted || referenceDeleted) {dbc->updateDeleteDataSet(nodeId);}
					}	
				}
			}
			//Publish DataboardClients that have changes
			for(std::shared_ptr<Umati::Dashboard::DashboardClient> dbc : changedDbcs) {
				std::unique_lock<decltype(m_dashboardClients_mutex)> ul(m_dashboardClients_mutex);
				dbc->Publish();
			}
		}
		
		void DashboardMachineObserver::AddSubscription()
		{
			dbmo = this;
			std::shared_ptr<UA_Client> client = m_pDataClient->getUaClient();
			UA_CreateSubscriptionRequest request = UA_CreateSubscriptionRequest_default();
    		UA_CreateSubscriptionResponse response = UA_Client_Subscriptions_create(client.get(), request,NULL, NULL, NULL);
			if(response.responseHeader.serviceResult == UA_STATUSCODE_GOOD) 
			{
				LOG(INFO) << "###################################### Created Subscription #####################################";
				UA_UInt32 subId = response.subscriptionId;
				UA_MonitoredItemCreateRequest item;
    			UA_MonitoredItemCreateRequest_init(&item);
				item.itemToMonitor.attributeId = UA_ATTRIBUTEID_EVENTNOTIFIER;
				item.monitoringMode = UA_MONITORINGMODE_REPORTING;
				//Allways Subscribe to the Serer object
    			item.itemToMonitor.nodeId = UA_NODEID_NUMERIC(0, 2253); // Root->Objects->Server
    			
    			

				UA_EventFilter filter;
    			UA_EventFilter_init(&filter);
				//Setup selection Clauses for Filter
				int nSelectClauses = 3;
				UA_SimpleAttributeOperand *selectClauses = (UA_SimpleAttributeOperand*)
        		UA_Array_new(nSelectClauses, &UA_TYPES[UA_TYPES_SIMPLEATTRIBUTEOPERAND]);

    			

				for(size_t i =0; i<nSelectClauses; ++i) {
        			UA_SimpleAttributeOperand_init(&selectClauses[i]);
    			}

				selectClauses[0].typeDefinitionId = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEEVENTTYPE);
				selectClauses[0].attributeId = UA_ATTRIBUTEID_VALUE;
    			selectClauses[0].browsePathSize = 1;
    			selectClauses[0].browsePath = (UA_QualifiedName*)
       			UA_Array_new(selectClauses[0].browsePathSize, &UA_TYPES[UA_TYPES_QUALIFIEDNAME]);
    			if(!selectClauses[0].browsePath) {
        			UA_SimpleAttributeOperand_delete(selectClauses);
    			}

    			selectClauses[0].browsePath[0] = UA_QUALIFIEDNAME_ALLOC(0, "EventType");

				selectClauses[1].typeDefinitionId = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEEVENTTYPE);
				selectClauses[1].attributeId = UA_ATTRIBUTEID_VALUE;
    			selectClauses[1].browsePathSize = 1;
    			selectClauses[1].browsePath = (UA_QualifiedName*)
       			UA_Array_new(selectClauses[1].browsePathSize, &UA_TYPES[UA_TYPES_QUALIFIEDNAME]);
    			if(!selectClauses[1].browsePath) {
        			UA_SimpleAttributeOperand_delete(selectClauses);
    			}

    			selectClauses[1].browsePath[0] = UA_QUALIFIEDNAME_ALLOC(0, "SourceNode");

				selectClauses[2].typeDefinitionId = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEEVENTTYPE);
				selectClauses[2].attributeId = UA_ATTRIBUTEID_VALUE;
    			selectClauses[2].browsePathSize = 1;
    			selectClauses[2].browsePath = (UA_QualifiedName*)
       			UA_Array_new(selectClauses[2].browsePathSize, &UA_TYPES[UA_TYPES_QUALIFIEDNAME]);
    			if(!selectClauses[2].browsePath) {
        			UA_SimpleAttributeOperand_delete(selectClauses);
    			}

    			selectClauses[2].browsePath[0] = UA_QUALIFIEDNAME_ALLOC(0, "Changes");    

				filter.selectClauses = selectClauses;
    			filter.selectClausesSize = 3;

				// Content Filter
				UA_ContentFilter contentFilter;
				UA_ContentFilter_init(&contentFilter);

				UA_ContentFilterElement contentFilterElement;
				UA_ContentFilterElement_init(&contentFilterElement);

				UA_LiteralOperand literalOperand;
				UA_LiteralOperand_init(&literalOperand);
				UA_Variant v;
				UA_NodeId generalModelChangeEventNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_GENERALMODELCHANGEEVENTTYPE);
				UA_Variant_setScalar(&v, &generalModelChangeEventNodeId, &UA_TYPES[UA_TYPES_NODEID]);
				literalOperand.value = v;

				contentFilterElement.filterOperator = UA_FILTEROPERATOR_OFTYPE;
				contentFilterElement.filterOperandsSize = 1;
				contentFilterElement.filterOperands = (UA_ExtensionObject*)
				UA_Array_new(contentFilterElement.filterOperandsSize, &UA_TYPES[UA_TYPES_EXTENSIONOBJECT]);
				UA_ExtensionObject extensionObject;
				UA_ExtensionObject_setValue(&extensionObject, &literalOperand, &UA_TYPES[UA_TYPES_LITERALOPERAND]);
				contentFilterElement.filterOperands[0] = extensionObject;

				contentFilter.elementsSize = 1;
				contentFilter.elements = (UA_ContentFilterElement* ) UA_Array_new(contentFilter.elementsSize, &UA_TYPES[UA_TYPES_CONTENTFILTERELEMENT]);
				contentFilter.elements[0] = contentFilterElement;
				filter.whereClause = contentFilter;


    			item.requestedParameters.filter.encoding = UA_EXTENSIONOBJECT_DECODED;
    			item.requestedParameters.filter.content.decoded.data = &filter;
    			item.requestedParameters.filter.content.decoded.type = &UA_TYPES[UA_TYPES_EVENTFILTER];

				UA_UInt32 monId = 0;

    			UA_MonitoredItemCreateResult result = UA_Client_MonitoredItems_createEvent(client.get(), subId, UA_TIMESTAMPSTORETURN_BOTH, item, &monId, handler_events, NULL);
				if(result.statusCode == UA_STATUSCODE_GOOD) {
       				 LOG(INFO) << "###################################### Created MonitoredItem #####################################";
				} else {
					LOG(INFO) << "###################################### Error Error Error #####################################";
				}
				monId = result.monitoredItemId; 
    		} else {
				UA_Client_disconnect(client.get());
        		UA_Client_delete(client.get());
			}		
		}
		void DashboardMachineObserver::PublishAll()
		{
			{
				std::unique_lock<decltype(m_dashboardClients_mutex)> ul(m_dashboardClients_mutex);
				for (const auto &pDashClient : m_dashboardClients)
				{
					pDashClient.second->Publish();
				}
			}

			// Publish online machines every 30th publish
			if (++m_publishMachinesOnline >= 30)
			{
				this->publishMachinesList();
				m_publishMachinesOnline = 0;
			}

		}

		void DashboardMachineObserver::startUpdateMachineThread()
		{
			if (m_running)
			{
				LOG(INFO) << "Machine update thread already running";
				return;
			}

			auto func = [this]() {
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
			std::unique_lock<decltype(m_dashboardClients_mutex)> ul_machines(m_dashboardClients_mutex);
			std::unique_lock<decltype(m_machineIdentificationsCache_mutex)> ul(m_machineIdentificationsCache_mutex);
			PublishMachinesList pubList(m_pPublisher, m_pOpcUaTypeReader->m_expectedObjectTypeNames, Topics::List);
			for (auto &machineOnline : m_onlineMachines)
			{
				auto it = m_machineIdentificationsCache.find(machineOnline.first);
				if(it == m_machineIdentificationsCache.end() || it->second.empty()) {
					continue;
				}
				auto identificationAsJson = it->second;
				/// \todo Refactor out of here
				identificationAsJson["ParentId"] = Umati::Util::IdEncode(static_cast<std::string>(machineOnline.second.Parent));
				pubList.AddMachine(machineOnline.second.Specification, identificationAsJson);
			}
            pubList.Publish();
            auto errors = std::vector<std::string>{"errors"};
            PublishMachinesList pubInvalidList(m_pPublisher, errors, Topics::ErrorList);
            for (auto &machineInvalid: m_invalidMachines)
            {
                auto it = m_machineIdentificationsCache.find(machineInvalid.first);
                if(it == m_machineIdentificationsCache.end() || it->second.empty()) {
                    continue;
                }
                auto identificationAsJson = it->second;
                identificationAsJson["Error"] = machineInvalid.second.second;
                pubInvalidList.AddMachine("errors", identificationAsJson);
            }
            pubInvalidList.Publish();
		}

		std::string DashboardMachineObserver::getTypeName(const ModelOpcUa::NodeId_t &nodeId)
		{
			return m_pDataClient->readNodeBrowseName(const_cast<ModelOpcUa::NodeId_t &>(nodeId));
		}

		void DashboardMachineObserver::addMachine(ModelOpcUa::BrowseResult_t machine)
		{
			std::string InputMachine = static_cast<std::string>(machine.NodeId);
			LOG(INFO) << "############################## InputMachine: " << InputMachine;
			//std::string myMachine = "nsu=de.uni-stuttgart.isw.glas.sampleserver;i=55091";
			//std::string myMachine = "nsu=de.uni-stuttgart.isw.glas.sampleserver;i=58192";
			//if(myMachine.compare(InputMachine) == 0) {
				try
				{
					LOG(INFO) << "New Machine: " << machine.BrowseName.Name << " NodeId:"
							<< static_cast<std::string>(machine.NodeId);
					auto pDashClient = std::make_shared<Umati::Dashboard::DashboardClient>(m_pDataClient, m_pPublisher, m_pOpcUaTypeReader);
					MachineInformation_t machineInformation;
					machineInformation.NamespaceURI = machine.NodeId.Uri;
					machineInformation.StartNodeId = machine.NodeId;
					machineInformation.MachineName = machine.BrowseName.Name;
					machineInformation.TypeDefinition = machine.TypeDefinition;

					{
						auto it = m_parentOfMachine.find(machine.NodeId);
						if(it != m_parentOfMachine.end())
						{
							machineInformation.Parent = it->second;
						}
					}

					std::shared_ptr<ModelOpcUa::StructureNode> p_type = m_pOpcUaTypeReader->typeDefinitionToStructureNode(machine.TypeDefinition);
					machineInformation.Specification = p_type->SpecifiedBrowseName.Name;
					auto it = m_dashboardClients.find(machine.NodeId);
					if (it != m_dashboardClients.end())
					{
						it->second->Unsubscribe(machine.NodeId);
						m_dashboardClients.erase(it);
						LOG(INFO) << "Removed Machine with duplicated reference to parent with NodeId:"
								<< static_cast<std::string>(machine.NodeId);
					}
					std::time_t t= std::time(0);
					LOG(INFO) << "Read Machines";
					pDashClient->addDataSet(
						{machineInformation.NamespaceURI, machine.NodeId.Id},
						p_type,
						Topics::Machine(p_type, static_cast<std::string>(machine.NodeId)));

					LOG(INFO) << "Read model finished";
					LOG(INFO) << "Time: " << std::time(0) - t;

					{
						std::unique_lock<decltype(m_dashboardClients_mutex)> ul(m_dashboardClients_mutex);
						m_dashboardClients.insert(std::make_pair(machine.NodeId, pDashClient));
						m_onlineMachines.insert(std::make_pair(machine.NodeId, machineInformation));
						m_machineNames.insert(std::make_pair(machine.NodeId, machine.BrowseName.Name));
					}

				}
				catch (const Umati::Exceptions::OpcUaException &ex)
				{
					LOG(ERROR) << "Could not add Machine " << machine.BrowseName.Name
							<< " NodeId:" << static_cast<std::string>(machine.NodeId) << "OpcUa Error: " << ex.what();

					throw Exceptions::MachineInvalidException(ex.what());
				}
			//}
		}

		void DashboardMachineObserver::removeMachine(ModelOpcUa::NodeId_t machineNodeId)
		{
			std::unique_lock<decltype(m_dashboardClients_mutex)> ul(m_dashboardClients_mutex);
			m_knownMachines.erase(machineNodeId);

			LOG(INFO) << "Remove Machine with NodeId:"
					  << static_cast<std::string>(machineNodeId);
			auto it = m_dashboardClients.find(machineNodeId);
			if (it != m_dashboardClients.end())
			{
				it->second.get()->Unsubscribe(machineNodeId);
				m_dashboardClients.erase(it);
			}
			else
			{
				LOG(INFO) << "Machine not known: '" << static_cast<std::string>(machineNodeId) << "'";
			}

			auto itOnlineMachines = m_onlineMachines.find(machineNodeId);
			if (itOnlineMachines != m_onlineMachines.end())
			{
				m_onlineMachines.erase(itOnlineMachines);
				LOG(INFO) << "Online machine erased";
			}
			else
			{
				LOG(INFO) << "Machine was not online: '" << static_cast<std::string>(machineNodeId) << "'";
			}
		}

		bool
		DashboardMachineObserver::isOnline(
			const ModelOpcUa::NodeId_t &machineNodeId,
			nlohmann::json &identificationAsJson,
			const ModelOpcUa::NodeId_t &typeDefinition)
		{
			try
			{	
				std::shared_ptr<ModelOpcUa::StructureNode> p_type = m_pOpcUaTypeReader->getIdentificationTypeStructureNode(typeDefinition);
				std::string typeName = p_type->SpecifiedBrowseName.Uri + ";" + p_type->SpecifiedBrowseName.Name;
				auto typeIt = m_pOpcUaTypeReader->m_nameToId->find(typeName);
				/// \todo Should be p_Type.specifiedTypeId ?
				if (typeIt != m_pOpcUaTypeReader->m_nameToId->end())
				{
					ModelOpcUa::NodeId_t type = typeIt->second;

					std::list<ModelOpcUa::BrowseResult_t> identification =
						m_pDataClient->BrowseHasComponent(machineNodeId, type);
					if (!identification.empty())
					{
						LOG(DEBUG) << "Found component of type " << type.Uri << ";" << type.Id << " in "
								   << machineNodeId.Uri << ";" << machineNodeId.Id;

						browseIdentificationValues(machineNodeId, typeDefinition, identification.front(),
												   identificationAsJson);
						if (!identificationAsJson.empty())
						{
							return true;
						}
						else
						{
							LOG(DEBUG) << "Identification JSON empty";
						}
					}
					else
					{
						LOG(INFO) << "Identification empty, couldn't find component of type " << type.Uri << ";"
								  << type.Id << " in " << machineNodeId.Uri << ";" << machineNodeId.Id;
					}
				}
				else
				{
					LOG(INFO) << "Unable to find type " << typeName << " in nameToId";
				}
			}
			catch (std::exception &ex)
			{
				LOG(ERROR) << ex.what();
			}
			return false;
		}


		void DashboardMachineObserver::browseIdentificationValues(const ModelOpcUa::NodeId_t &machineNodeId, 
																  const ModelOpcUa::NodeId_t &typeDefinition,
																  ModelOpcUa::BrowseResult_t &identification,
																  nlohmann::json &identificationAsJson) const
		{
			std::vector<nlohmann::json> identificationListValues;
			std::shared_ptr<ModelOpcUa::StructureNode> p_type = m_pOpcUaTypeReader->typeDefinitionToStructureNode(typeDefinition);			
			std::list<ModelOpcUa::NodeId_t> identificationNodes;
			std::vector<std::string> identificationValueKeys;

			FillIdentificationValuesFromBrowseResult(
				identification.NodeId,
				identificationNodes,
				identificationValueKeys);

			nlohmann::json identificationData;
			identificationListValues = m_pDataClient->ReadeNodeValues(identificationNodes);
			for (size_t i = 0; i < identificationListValues.size(); i++)
			{
				auto value = identificationListValues.at(i);
				if (value != nullptr)
				{
					identificationData[identificationValueKeys.at(i)] = value;
				}
			}

			identificationAsJson["Data"] = identificationData;

			auto it = m_machineNames.find(machineNodeId);
			if (it != m_machineNames.end() && p_type != nullptr)
			{
				identificationAsJson["Topic"] = Topics::Machine(p_type, static_cast<std::string>(machineNodeId));
				identificationAsJson["MachineId"] = Umati::Util::IdEncode(static_cast<std::string>(machineNodeId));
				identificationAsJson["TypeDefinition"] = p_type->SpecifiedBrowseName.Name;
			}
		}


		void DashboardMachineObserver::FillIdentificationValuesFromBrowseResult(
			const ModelOpcUa::NodeId_t &identificationInstance,
			std::list<ModelOpcUa::NodeId_t> &identificationNodes,
			std::vector<std::string> &identificationValueKeys) const
		{
			///\TODO browse by type definition
			auto browseResults = m_pDataClient->Browse(
				identificationInstance,
				Umati::Dashboard::IDashboardDataClient::BrowseContext_t::Variable());
			for (auto &browseResult : browseResults)
			{
				identificationValueKeys.push_back(browseResult.BrowseName.Name);
				identificationNodes.emplace_back(browseResult.NodeId);
			}
		}
		
	} // namespace MachineObserver
} // namespace Umati
