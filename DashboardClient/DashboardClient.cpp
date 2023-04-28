 /* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 * 
 * Copyright 2019-2022 (c) Christian von Arnim, ISW University of Stuttgart (for umati and VDW e.V.)
 * Copyright 2020 (c) Dominik Basner, Sotec GmbH (for VDW e.V.)
 * Copyright 2021 (c) Moritz Walker, ISW University of Stuttgart (for umati and VDW e.V.)
 * Copyright 2021 (c) Marius Dege, basysKom GmbH
 */

#include "DashboardClient.hpp"

#include <easylogging++.h>
#include <Exceptions/OpcUaException.hpp>
#include "Converter/ModelToJson.hpp"

namespace Umati
{

	namespace Dashboard
	{

		DashboardClient::DashboardClient(
			std::shared_ptr<IDashboardDataClient> pDashboardDataClient,
			std::shared_ptr<IPublisher> pPublisher,
			std::shared_ptr<OpcUaTypeReader> pTypeReader)
			: m_pDashboardDataClient(pDashboardDataClient), m_pPublisher(pPublisher), m_pTypeReader(pTypeReader)
		{
		}


		/**
		* Receives a nodeId, a typeDefinition and an mqtt topic to hold for a machine. Available types are
		* Identification, JobCurrentStateNumber, ProductionJobList, Stacklight, StateModelList, ToolList
		*/
		void DashboardClient::addDataSet(
			const ModelOpcUa::NodeId_t &startNodeId,
			const std::shared_ptr<ModelOpcUa::StructureNode> &pTypeDefinition,
			const std::string &channel,
			const std::string &onlineChannel
			)
		{
			try
			{
				std::shared_ptr<DataSetStorage_t> pDataSetStorage = prepareDataSetStorage(
					startNodeId,
					pTypeDefinition,
					channel,
					onlineChannel);
				LOG(INFO) << "DataSetStorage prepared for " << channel;
				subscribeValues(pDataSetStorage->node, pDataSetStorage->values, pDataSetStorage->values_mutex);
				LOG(INFO) << "Values subscribed for  " << channel;
				std::lock_guard<std::recursive_mutex> l(m_dataSetMutex);
				m_dataSets.push_back(pDataSetStorage);
			}
			catch (const Umati::Exceptions::OpcUaException &ex)
			{
				LOG(WARNING) << ex.what();
			}
            catch (MachineObserver::Exceptions::MachineInvalidChildException &ex)
            {
                LOG(ERROR) << ex.what();
                throw Exceptions::OpcUaException(ex.what());
            }
			catch (std::exception &ex)
			{
				LOG(ERROR) << ex.what();
                throw Exceptions::OpcUaException(ex.what());
			}
		}

		std::shared_ptr<DashboardClient::DataSetStorage_t>
		DashboardClient::prepareDataSetStorage(const ModelOpcUa::NodeId_t &startNodeId,
											   const std::shared_ptr<ModelOpcUa::StructureNode> &pTypeDefinition,
											   const std::string &channel,
											   const std::string &onlineChannel)
		{
			auto pDataSetStorage = std::make_shared<DataSetStorage_t>();
			pDataSetStorage->startNodeId = startNodeId;
			pDataSetStorage->channel = channel;
			pDataSetStorage->onlineChannel = onlineChannel;
			pDataSetStorage->node = TransformToNodeIds(startNodeId, pTypeDefinition);
			return pDataSetStorage;
		}

		void DashboardClient::Publish()
		{
			std::lock_guard<std::recursive_mutex> l(m_dataSetMutex);
			for (auto &pDataSetStorage : m_dataSets)
			{
				std::string jsonPayload = getJson(pDataSetStorage);
				if (!jsonPayload.empty() && jsonPayload != "null")
				{
					LastMessage_t &lastMessage = m_latestMessages[pDataSetStorage->channel];

					time_t now;
					time(&now);

					if (jsonPayload != lastMessage.payload || difftime(now, lastMessage.lastSent) > 10)
					{
						m_pPublisher->Publish(pDataSetStorage->channel, jsonPayload);
						lastMessage.payload = jsonPayload;
						lastMessage.lastSent = now;
					}
					m_pPublisher->Publish(pDataSetStorage->onlineChannel, "1");
				}
				else
				{
					LOG(INFO) << "pdatasetstorage for " << pDataSetStorage->startNodeId.Uri << ";"
							  << pDataSetStorage->startNodeId.Id << " is empty";
				}
			}
		}

		void DashboardClient::Unsubscribe(ModelOpcUa::NodeId_t nodeId){

			std::vector<int32_t> monItemIds;
			std::vector<int32_t> clientHandles;
			for (auto values : m_subscribedValues){

				auto value = values.get();

				if(value){
					monItemIds.push_back(value->getMonitoredItemId());
					clientHandles.push_back(value->getClientHandle());
				}else{
					LOG(ERROR) << "Monitored Item is NULL for NodeId: " << nodeId.Id;
				}
				
			}
			m_subscribedValues.clear();

			m_pDashboardDataClient->Unsubscribe(monItemIds, clientHandles);

			std::lock_guard<std::recursive_mutex> l(m_dataSetMutex);
			m_dataSets.clear();
			
		}

		std::string DashboardClient::getJson(const std::shared_ptr<DataSetStorage_t> &pDataSetStorage)
		{
			auto getValueCallback = [pDataSetStorage](
										const std::shared_ptr<const ModelOpcUa::Node> &pNode) -> nlohmann::json {
				std::unique_lock<decltype(pDataSetStorage->values_mutex)> ul(pDataSetStorage->values_mutex);
				auto it = pDataSetStorage->values.find(pNode);
				if (it == pDataSetStorage->values.end()) {
					LOG(DEBUG) << "Couldn't write value for " << pNode->SpecifiedBrowseName.Name << " | " << pNode->SpecifiedTypeNodeId.Uri << ";" << pNode->SpecifiedTypeNodeId.Id << "Try to search it with NodeId!";
					// In case we don't wnt to remove the duplicate pointers with FIX_1, we can simply check the
					// Identity of the node via its node Id.
					auto pSimpleNode = std::dynamic_pointer_cast<const ModelOpcUa::SimpleNode>(pNode);
					if(pSimpleNode) {
						LOG(DEBUG) << pSimpleNode->NodeId << "\n";
						auto values = pDataSetStorage->values;
						for(auto it1 : values) {
							auto pSimpleNode1 = std::dynamic_pointer_cast<const ModelOpcUa::SimpleNode>(it1.first);
								if(pSimpleNode1->NodeId == pSimpleNode->NodeId) {
									// DEBUG_BEGIN in case we want to see the different pointer addresses.
									LOG(DEBUG) << pSimpleNode.get() << "\n";
									LOG(DEBUG) << pSimpleNode1.get() << "\n";
									LOG(DEBUG) << pNode->SpecifiedBrowseName.Name << " " << "found!";
									return it1.second;
								}
						}
					}
					LOG(DEBUG) << pNode->SpecifiedBrowseName.Name << " " << " not found!";
					return nullptr;
				}
				return it->second;
			};

			return Converter::ModelToJson(pDataSetStorage->node, getValueCallback).getJson().dump(2);
		}

		std::shared_ptr<const ModelOpcUa::SimpleNode> DashboardClient::TransformToNodeIds(
			ModelOpcUa::NodeId_t startNode,
			const std::shared_ptr<ModelOpcUa::StructureNode> &pTypeDefinition)
		{
			auto ret = browsedNodes.insert(startNode);

			// FIX_BEGIN (FIX_1), removed duplicates to avoid different pointers to the same node
			(*pTypeDefinition->SpecifiedChildNodes).sort();
			(*pTypeDefinition->SpecifiedChildNodes).unique();
			// FIX_END
			std::list<std::shared_ptr<const ModelOpcUa::Node>> foundChildNodes;
			if(ret.second==true) {
				for (auto &pChild : *pTypeDefinition->SpecifiedChildNodes)
				{
					switch (pChild->ModellingRule)
					{
					case ModelOpcUa::ModellingRule_t::Optional:
					case ModelOpcUa::ModellingRule_t::Mandatory:
					{
						bool should_continue = OptionalAndMandatoryTransformToNodeId(
							startNode,
							foundChildNodes,
							pChild);
						if (should_continue)
						{
							continue;
						}
						break;
					}
					case ModelOpcUa::ModellingRule_t::OptionalPlaceholder:
					case ModelOpcUa::ModellingRule_t::MandatoryPlaceholder:
					{
						bool should_continue = OptionalAndMandatoryPlaceholderTransformToNodeId(
							startNode,
							foundChildNodes,
							pChild);
						if (should_continue)
						{
							continue;
						}
						break;
					}
					case ModelOpcUa::ModellingRule_t::None:
					{
						LOG(INFO) << "modelling rule is none";
						LOG(ERROR) << "Unknown Modelling Rule None." << std::endl;
						break;
					}
					default:
						LOG(ERROR) << "Unknown Modelling Rule." << std::endl;
						break;
					}
				}
			}
			auto pNode = std::make_shared<ModelOpcUa::SimpleNode>(
				startNode,
				pTypeDefinition->SpecifiedTypeNodeId,
				*pTypeDefinition,
				foundChildNodes);

			pNode->ofBaseDataVariableType = pTypeDefinition->ofBaseDataVariableType;
			return pNode;
		}

        void LogOptionalAndMandatoryTransformToNodeIdError(const ModelOpcUa::NodeId_t &nodeId, const ModelOpcUa::QualifiedName_t &childBrowsName, const char *err) {
            LOG(ERROR) << "Forwarding exception, cause:"
                       << "Could not find '"
                       << static_cast<std::string>(nodeId)
                       << "'->'"
                       << static_cast<std::string>(childBrowsName)
                       << "'"
                       << "Unknown ID caused exception: " << err;
        }

        std::string GetOptionalAndMandatoryTransformToNodeIdError(const ModelOpcUa::NodeId_t &nodeId, const ModelOpcUa::QualifiedName_t &childBrowseName, const char *err) {
            return "In '" + static_cast<std::string>(nodeId)
                   + "'->'"
                   + static_cast<std::string>(childBrowseName)
                   + "':\n" + err;
        }

		/**
		 * @return if the switch case should break
		 */
		bool DashboardClient::OptionalAndMandatoryTransformToNodeId(const ModelOpcUa::NodeId_t &startNode,
																	std::list<std::shared_ptr<const ModelOpcUa::Node>> &foundChildNodes,
																	const std::shared_ptr<ModelOpcUa::StructureNode> &pChild)
		{
			try
			{
				auto childNodeId = m_pDashboardDataClient->TranslateBrowsePathToNodeId(startNode,
																					   pChild->SpecifiedBrowseName);
				if (childNodeId.isNull())
				{
					TransformToNodeIdNodeNotFoundLog(startNode, pChild);
					return false;
				}
				auto nodeIds = TransformToNodeIds(childNodeId, pChild);
				foundChildNodes.push_back(nodeIds);
			}
            catch (MachineObserver::Exceptions::MachineInvalidChildException &ex)
            {
                if (ex.hasInvalidMandatoryChild)
                {
                    std::string err = GetOptionalAndMandatoryTransformToNodeIdError(startNode, pChild->SpecifiedBrowseName, ex.what());
                    LogOptionalAndMandatoryTransformToNodeIdError(startNode, pChild->SpecifiedBrowseName, ex.what());
                    throw MachineObserver::Exceptions::MachineInvalidChildException(err, ex.hasInvalidMandatoryChild);
                }
                return false;
            }
			catch (std::exception &ex)
			{
				if (pChild->ModellingRule != ModelOpcUa::ModellingRule_t::Optional)
				{
                    std::string err = GetOptionalAndMandatoryTransformToNodeIdError(startNode, pChild->SpecifiedBrowseName, ex.what());
                    LogOptionalAndMandatoryTransformToNodeIdError(startNode, pChild->SpecifiedBrowseName, ex.what());
                    throw MachineObserver::Exceptions::MachineInvalidChildException(err, true);
				}
				return false;
			}
			return true;
		}

		void DashboardClient::TransformToNodeIdNodeNotFoundLog(const ModelOpcUa::NodeId_t &startNode,
															   const std::shared_ptr<ModelOpcUa::StructureNode> &pChild) const
		{
			LOG(INFO) << "Could not find '"
					  << static_cast<std::string>(startNode)
					  << "'->'"
					  << static_cast<std::string>(pChild->SpecifiedBrowseName)
					  << "'";
		}

		bool DashboardClient::OptionalAndMandatoryPlaceholderTransformToNodeId(const ModelOpcUa::NodeId_t &startNode,
																			   std::list<std::shared_ptr<const ModelOpcUa::Node>> &foundChildNodes,
																			   const std::shared_ptr<ModelOpcUa::StructureNode> &pChild)
		{
			try
			{
				const ModelOpcUa::StructurePlaceholderNode structurePChild(pChild);
				auto pPlaceholderChild = std::make_shared<const ModelOpcUa::StructurePlaceholderNode>(structurePChild);

				if (!pPlaceholderChild)
				{
					LOG(ERROR) << "Child " << pChild->SpecifiedBrowseName.Uri << ";" << pChild->SpecifiedBrowseName.Name
							   << " of " << startNode.Uri << ";" << startNode.Id
							   << " caused a Placeholder error: instance not a placeholder." << std::endl;
					return true;
				}
				auto placeholderNode = BrowsePlaceholder(startNode, pPlaceholderChild);
				foundChildNodes.push_back(placeholderNode);
			}
			catch (std::exception &ex)
			{
				LOG(ERROR) << "Child " << pChild->SpecifiedBrowseName.Uri << ";" << pChild->SpecifiedBrowseName.Name
						   << " of " << startNode.Uri << ";" << startNode.Id
						   << " caused an exception: Unknown ID caused exception: " << ex.what();
				if (pChild->ModellingRule != ModelOpcUa::ModellingRule_t::OptionalPlaceholder)
				{
					LOG(ERROR) << "Forwarding exception, cause:"
							   << "Could not find '"
							   << static_cast<std::string>(startNode)
							   << "'->'"
							   << static_cast<std::string>(pChild->SpecifiedBrowseName)
							   << "'"
							   << "Unknown ID caused exception: " << ex.what();
					throw ex;
				}
				return false;
			}
			return true;
		}

		std::shared_ptr<const ModelOpcUa::PlaceholderNode> DashboardClient::BrowsePlaceholder(
			ModelOpcUa::NodeId_t startNode,
			std::shared_ptr<const ModelOpcUa::StructurePlaceholderNode> pStructurePlaceholder)
		{
			if (!pStructurePlaceholder)
			{
				LOG(ERROR) << "Invalid Argument, pStructurePlaceholder is nullptr";
				throw std::invalid_argument("pStructurePlaceholder is nullptr.");
			}

			auto pPlaceholderNode = std::make_shared<ModelOpcUa::PlaceholderNode>(
				*pStructurePlaceholder,
				std::list<std::shared_ptr<const ModelOpcUa::Node>>{});
			auto browseResults = m_pDashboardDataClient->Browse(startNode, pStructurePlaceholder->ReferenceType,
																pStructurePlaceholder->SpecifiedTypeNodeId);

			preparePlaceholderNodesTypeId(pStructurePlaceholder, pPlaceholderNode, browseResults);

			return pPlaceholderNode;
		}

		void DashboardClient::preparePlaceholderNodesTypeId(
			const std::shared_ptr<const ModelOpcUa::StructurePlaceholderNode> & pStructurePlaceholder,
			std::shared_ptr<ModelOpcUa::PlaceholderNode> &pPlaceholderNode,
			std::list<ModelOpcUa::BrowseResult_t> &browseResults)
		{
			for (auto &browseResult : browseResults)
			{	
				if (browseResult.TypeDefinition.Id == NodeId_BaseObjectType.Id) {
					auto ifs = m_pDashboardDataClient->Browse(browseResult.NodeId,
						Dashboard::IDashboardDataClient::BrowseContext_t::HasInterface());
						browseResult.TypeDefinition = ifs.front().NodeId;
						LOG(INFO) << "Updated TypeDefinition of " << browseResult.BrowseName.Name << " to " << browseResult.TypeDefinition 
								  << " because the node implements an interface";				
				}
				auto possibleType = m_pTypeReader->m_typeMap->find(browseResult.TypeDefinition);  // use subtype
				if (possibleType != m_pTypeReader->m_typeMap->end())
				{
					// LOG(INFO) << "Found type for " << typeName;
					auto sharedPossibleType = possibleType->second;
					ModelOpcUa::PlaceholderElement plElement;
					plElement.BrowseName = browseResult.BrowseName;
					plElement.pNode = TransformToNodeIds(browseResult.NodeId, sharedPossibleType);
                    plElement.TypeDefinition = browseResult.TypeDefinition;
					pPlaceholderNode->addInstance(plElement);
				}
				else
				{
					LOG(WARNING) << "Could not find a possible type for "
								 << static_cast<std::string>(browseResult.TypeDefinition)
								 << ". Continuing without a candidate.";
					//LOG(WARNING) << "Pointer shows to end()";
				}
			}
		}

		void DashboardClient::subscribeValues(
			const std::shared_ptr<const ModelOpcUa::SimpleNode> pNode,
			std::map<std::shared_ptr<const ModelOpcUa::Node>, nlohmann::json> &valueMap,
			std::mutex &valueMap_mutex)
		{
			// LOG(INFO) << "subscribeValues "   << pNode->NodeId.Uri << ";" << pNode->NodeId.Id;

			// Only Mandatory/Optional variables
			if (isMandatoryOrOptionalVariable(pNode))
			{	
				subscribeValue(pNode, valueMap, valueMap_mutex);
				
			}
			handleSubscribeChildNodes(pNode, valueMap, valueMap_mutex);
		}

		void DashboardClient::handleSubscribeChildNodes(const std::shared_ptr<const ModelOpcUa::SimpleNode> &pNode,
														std::map<std::shared_ptr<const ModelOpcUa::Node>, nlohmann::json> &valueMap,
														std::mutex &valueMap_mutex)
		{
			// LOG(INFO) << "handleSubscribeChildNodes "   << pNode->NodeId.Uri << ";" << pNode->NodeId.Id;
			if (pNode->ChildNodes.size() == 0)
			{
				// LOG(INFO) << "No children found for "  << pNode->NodeId.Uri << ";" << pNode->NodeId.Id;
			}
			for (auto &pChildNode : pNode->ChildNodes)
			{
				switch (pChildNode->ModellingRule)
				{
				case ModelOpcUa::Mandatory:
				case ModelOpcUa::Optional:
				{
					handleSubscribeChildNode(pChildNode, valueMap, valueMap_mutex);
					break;
				}
				case ModelOpcUa::MandatoryPlaceholder:
				case ModelOpcUa::OptionalPlaceholder:
				{
					handleSubscribePlaceholderChildNode(pChildNode, valueMap, valueMap_mutex);
					break;
				}
				default:
				{
					LOG(ERROR) << "Unknown Modelling Rule." << std::endl;
					break;
				}
				}
			}
		}

		void DashboardClient::handleSubscribeChildNode(const std::shared_ptr<const ModelOpcUa::Node> &pChildNode,
													   std::map<std::shared_ptr<const ModelOpcUa::Node>, nlohmann::json> &valueMap,
													   std::mutex &valueMap_mutex)
		{
			// LOG(INFO) << "handleSubscribeChildNode " <<  pChildNode->SpecifiedBrowseName.Uri << ";" <<  pChildNode->SpecifiedBrowseName.Name;

			auto pSimpleChild = std::dynamic_pointer_cast<const ModelOpcUa::SimpleNode>(pChildNode);
			if (!pSimpleChild)
			{
				LOG(ERROR) << "Simple node error, instance not a simple node." << std::endl;
				return;
			}
			// recursive call
			subscribeValues(pSimpleChild, valueMap, valueMap_mutex);
		}

		void
		DashboardClient::handleSubscribePlaceholderChildNode(const std::shared_ptr<const ModelOpcUa::Node> &pChildNode,
															 std::map<std::shared_ptr<const ModelOpcUa::Node>, nlohmann::json> &valueMap,
															 std::mutex &valueMap_mutex)
		{
			// LOG(INFO) << "handleSubscribePlaceholderChildNode " << pChildNode->SpecifiedBrowseName.Uri << ";" << pChildNode->SpecifiedBrowseName.Name;
			auto pPlaceholderChild = std::dynamic_pointer_cast<const ModelOpcUa::PlaceholderNode>(pChildNode);
			if (!pPlaceholderChild)
			{
				LOG(ERROR) << "Placeholder error, instance not a placeholder." << std::endl;
				return;
			}

			auto placeholderElements = pPlaceholderChild->getInstances();

			for (const auto &pPlaceholderElement : placeholderElements)
			{
				// recursive call
				subscribeValues(pPlaceholderElement.pNode, valueMap, valueMap_mutex);
			}
		}

		void DashboardClient::subscribeValue(const std::shared_ptr<const ModelOpcUa::SimpleNode> &pNode,
											 std::map<std::shared_ptr<const ModelOpcUa::Node>, nlohmann::json> &valueMap,
											 std::mutex &valueMap_mutex
											 )
		{ /**
                                             * Creates a lambda function which gets pNode as a copy and valueMap as a reference from this function,
                                             * the input parameters of the lambda function is the nlohmann::json value and the body updates the value
                                             * at position pNode with the received json value.
                                             */
			// LOG(INFO) << "SubscribeValue " << pNode->SpecifiedBrowseName.Uri << ";" << pNode->SpecifiedBrowseName.Name << " | " << pNode->NodeId.Uri << ";" << pNode->NodeId.Id;
			
			auto callback = [pNode, &valueMap, &valueMap_mutex](nlohmann::json value) {
					std::unique_lock<std::remove_reference<decltype(valueMap_mutex)>::type>(valueMap_mutex);
					valueMap[pNode] = value;
			};
			try
			{
				for(auto value : m_subscribedValues){
					if(value && value.get()->getNodeId() == pNode.get()->NodeId)
					return;
				}
				auto subscribedValue = m_pDashboardDataClient->Subscribe(pNode->NodeId, callback);
				m_subscribedValues.push_back(subscribedValue);
			}
			catch (std::exception &ex)
			{
				LOG(ERROR) << "Subscribe thrown an error: " << ex.what();
			}
		}

		bool
		DashboardClient::isMandatoryOrOptionalVariable(const std::shared_ptr<const ModelOpcUa::SimpleNode> &pNode)
		{
			return (pNode->NodeClass == ModelOpcUa::NodeClass_t::Variable ||
					pNode->NodeClass == ModelOpcUa::NodeClass_t::VariableType) &&
				   (pNode->ModellingRule == ModelOpcUa::ModellingRule_t::Mandatory || pNode->ModellingRule == ModelOpcUa::ModellingRule_t::Optional);
		}
	} // namespace Dashboard
} // namespace Umati
