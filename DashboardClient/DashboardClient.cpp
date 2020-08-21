#include "DashboardClient.hpp"

#include <easylogging++.h>
#include <Exceptions/OpcUaException.hpp>
#include "Converter/ModelToJson.hpp"


namespace Umati {

	namespace Dashboard {

		DashboardClient::DashboardClient(
			std::shared_ptr<IDashboardDataClient> pDashboardDataClient,
			std::shared_ptr<IPublisher> pPublisher
		)
			: m_pDashboardDataClient(pDashboardDataClient), m_pPublisher(pPublisher)
		{
		}

		DashboardClient::~DashboardClient()
		{
			// Ensure that everything is unsubscribed before deleting m_dataSets
			m_subscribedValues.clear();
		}

		/**
		* Receives a nodeId, a typeDefinition and an mqtt topic to hold for a machine. Available types are
		* Identification, JobCurrentStateNumber, ProductionJobList, Stacklight, StateModelList, ToolList
		*/
		void DashboardClient::addDataSet(
			ModelOpcUa::NodeId_t startNodeId,
			std::shared_ptr<ModelOpcUa::StructureNode> pTypeDefinition,
			std::string channel)
		{
		    try {
                std::shared_ptr<DataSetStorage_t> pDataSetStorage = prepareDataSetStorage(startNodeId, pTypeDefinition, channel);
                subscribeValues(pDataSetStorage->node, pDataSetStorage->values);
                m_dataSets.push_back(pDataSetStorage);
            }
		    catch(const Umati::Exceptions::OpcUaException &ex) {
		        LOG(WARNING) << ex.what();
		    }
		}

        std::shared_ptr<DashboardClient::DataSetStorage_t>
        DashboardClient::prepareDataSetStorage(const ModelOpcUa::NodeId_t &startNodeId,
                                               const std::shared_ptr<ModelOpcUa::StructureNode> &pTypeDefinition,
                                               const std::string &channel) {
            auto pDataSetStorage = std::make_shared<DataSetStorage_t>();
            pDataSetStorage->startNodeId = startNodeId;
            pDataSetStorage->channel = channel;
            pDataSetStorage->pTypeDefinition = pTypeDefinition;
            pDataSetStorage->node = TransformToNodeIds(startNodeId, pTypeDefinition);
            return pDataSetStorage;
        }

        void DashboardClient::Publish()
		{
			for (auto &pDataSetStorage : m_dataSets)
			{
			    std::string jsonPayload = getJson(pDataSetStorage);
			    if(!jsonPayload.empty() && jsonPayload != "null") {
                    auto it = m_latestMessages.find(pDataSetStorage->channel);
                    time_t now;
                    time(&now);
                    if (it == m_latestMessages.end()) {
                        m_pPublisher->Publish(pDataSetStorage->channel, jsonPayload);

                        LastMessage_t lastMessage;
                        lastMessage.payload = jsonPayload;
                        lastMessage.lastSent = now;
                        std::pair <std::string, LastMessage_t> currentMessage(pDataSetStorage->channel, lastMessage);
                        m_latestMessages.insert(currentMessage);
                    } else if (jsonPayload != it->second.payload || difftime(now, it->second.lastSent) > 60) {
                        m_pPublisher->Publish(pDataSetStorage->channel, jsonPayload);
                        it->second.payload = jsonPayload;
                        it->second.lastSent = now;
                    }
                } else {
			        LOG(INFO) << "pdatasetstorage for " << pDataSetStorage->startNodeId.Uri << ";" << pDataSetStorage->startNodeId.Id << " is empty";
			    }
			}
		}

		std::string DashboardClient::getJson(std::shared_ptr<DataSetStorage_t> pDataSetStorage)
		{
			auto getValueCallback = [pDataSetStorage](const std::shared_ptr<const ModelOpcUa::Node> pNode) -> nlohmann::json
			{
				auto it = pDataSetStorage->values.find(pNode);
				if (it == pDataSetStorage->values.end())
				{
				    LOG(INFO) << "Couldn't write value for " << pNode->SpecifiedBrowseName.Name << " | " << pNode->SpecifiedTypeNodeId.Uri << ";" << pNode->SpecifiedTypeNodeId.Id;
					return nullptr;
				}
                return it->second;
			};

			return Converter::ModelToJson(pDataSetStorage->node, getValueCallback).getJson().dump(2);
		}

		std::shared_ptr<const ModelOpcUa::SimpleNode> DashboardClient::TransformToNodeIds(
			ModelOpcUa::NodeId_t startNode,
			const std::shared_ptr<const ModelOpcUa::StructureNode> &pTypeDefinition
		)
		{
			std::list<std::shared_ptr<const ModelOpcUa::Node>> foundChildNodes;
			for (auto & pChild : pTypeDefinition->SpecifiedChildNodes)
			{
				switch (pChild->ModellingRule)
				{
				case ModelOpcUa::ModellingRule_t::Optional:
				case ModelOpcUa::ModellingRule_t::Mandatory:
				{
                    bool should_continue = OptionalAndMandatoryTransformToNodeId(startNode, foundChildNodes, pChild);
                    if (should_continue) {
                        continue;
                    }
                    break;
				}
				case ModelOpcUa::ModellingRule_t::OptionalPlaceholder:
				case ModelOpcUa::ModellingRule_t::MandatoryPlaceholder:
				{
                    bool should_continue = OptionalAndMandatoryPlaceholderTransformToNodeId(startNode, foundChildNodes, pChild);
                    if (should_continue) {
                        continue;
                    }
                    break;
				}
				case ModelOpcUa::ModellingRule_t::None: {
				    LOG(INFO) << "modelling rule is none";
				}
				default:
					LOG(ERROR) << "Unknown Modelling Rule." << std::endl;
					break;
				}
			}

			auto pNode = std::make_shared<ModelOpcUa::SimpleNode>(
				startNode,
				pTypeDefinition->SpecifiedTypeNodeId,
				*pTypeDefinition,
				foundChildNodes
				);
			return pNode;
		}

		/**
		 * @return if the switch case should break
		 */
		bool DashboardClient::OptionalAndMandatoryTransformToNodeId(const ModelOpcUa::NodeId_t &startNode,
                                                                    std::list<std::shared_ptr<const ModelOpcUa::Node>> &foundChildNodes,
                                                                    const std::shared_ptr<const ModelOpcUa::StructureNode> &pChild) {
		    try{
                auto childNodeId = m_pDashboardDataClient->TranslateBrowsePathToNodeId(startNode, pChild->SpecifiedBrowseName);
                if (childNodeId.isNull())
                {
                    TransformToNodeIdNodeNotFoundLog(startNode, pChild);
                    return false;
                }
                auto nodeIds = TransformToNodeIds(childNodeId, pChild);
                foundChildNodes.push_back(nodeIds);
            }
            catch(std::exception &ex){
                TransformToNodeIdNodeNotFoundLog(startNode, pChild);
                LOG(ERROR) << "Unknown ID caused exception: " << ex.what() << ". Exception will be forwarded if child node was mandatory";
                if(pChild->ModellingRule != ModelOpcUa::ModellingRule_t::Optional) {
                    LOG(ERROR) << "Forwarding exception";
                    throw ex;
                }
                return false;
            }
            return true;
		}

        void DashboardClient::TransformToNodeIdNodeNotFoundLog(const ModelOpcUa::NodeId_t &startNode,
                                                               const std::shared_ptr<const ModelOpcUa::StructureNode> &pChild) const {
            LOG(INFO) << "Could not find '"
              << static_cast<std::string>(startNode)
              << "'->'"
              << static_cast<std::string>(pChild->SpecifiedBrowseName)
              << "'";
        }

        bool DashboardClient::OptionalAndMandatoryPlaceholderTransformToNodeId(const ModelOpcUa::NodeId_t &startNode,
                                                                               std::list<std::shared_ptr<const ModelOpcUa::Node>> &foundChildNodes,
                                                                               const std::shared_ptr<const ModelOpcUa::StructureNode> &pChild) {
		    try {
                const ModelOpcUa::StructurePlaceholderNode structurePChild(pChild);
                auto pPlaceholderChild = std::make_shared<const ModelOpcUa::StructurePlaceholderNode>(structurePChild);

                if (!pPlaceholderChild) {
                    LOG(ERROR) << "Child " << pChild->SpecifiedBrowseName.Uri << ";" << pChild->SpecifiedBrowseName.Name << " of " << startNode.Uri << ";"<< startNode.Id << " caused a Placeholder error: instance not a placeholder." << std::endl;
                    return true;
                }
                auto placeholderNode = BrowsePlaceholder(startNode, pPlaceholderChild);
                foundChildNodes.push_back(placeholderNode);
            }
		    catch(std::exception &ex) {
                LOG(ERROR) << "Child " << pChild->SpecifiedBrowseName.Uri << ";" << pChild->SpecifiedBrowseName.Name << " of " << startNode.Uri << ";"<< startNode.Id << " caused an exception: Unknown ID caused exception: " << ex.what() << ". Exception will be forwarded if child node was mandatory";
                if(pChild->ModellingRule != ModelOpcUa::ModellingRule_t::OptionalPlaceholder) {
                    throw ex;
                }
                return false;
		    }
            return true;
        }

		std::shared_ptr<const ModelOpcUa::PlaceholderNode> DashboardClient::BrowsePlaceholder(
			ModelOpcUa::NodeId_t startNode,
			std::shared_ptr<const ModelOpcUa::StructurePlaceholderNode> pStructurePlaceholder
		)
		{
			if (!pStructurePlaceholder)
			{
				LOG(ERROR) << "Invalid Argument, pStructurePlaceholder is nullptr";
				throw std::invalid_argument("pStructurePlaceholder is nullptr.");
			}

			auto pPlaceholderNode = std::make_shared<ModelOpcUa::PlaceholderNode>(
				*pStructurePlaceholder,
				std::list<std::shared_ptr<const ModelOpcUa::Node>>{}
			);
			auto browseResults = m_pDashboardDataClient->Browse(startNode, pStructurePlaceholder->ReferenceType, pStructurePlaceholder->SpecifiedTypeNodeId);
            preparePlaceholderNodesTypeId(pStructurePlaceholder, pPlaceholderNode, browseResults);

            return pPlaceholderNode;
		}

        void DashboardClient::preparePlaceholderNodesTypeId(
                const std::shared_ptr<const ModelOpcUa::StructurePlaceholderNode> &pStructurePlaceholder,
                std::shared_ptr<ModelOpcUa::PlaceholderNode> &pPlaceholderNode,
                const std::list<ModelOpcUa::BrowseResult_t> &browseResults) {
            for (auto &browseResult : browseResults)
            {   // todo check if placeholder has subtype that we use
                // use actually used subtype std::string typeName = m_pDashboardDataClient->getTypeName(browseResult.TypeDefinition);
                std::string typeName = m_pDashboardDataClient->getTypeName(pStructurePlaceholder->SpecifiedTypeNodeId); // use basic subtype

                auto possibleType = m_pDashboardDataClient->m_typeMap->find(typeName);
                if(possibleType != m_pDashboardDataClient->m_typeMap->end()) {
                    auto sharedPossibleType = std::make_shared<ModelOpcUa::StructureNode>(possibleType->second);
                    ModelOpcUa::PlaceholderElement plElement;
                    plElement.BrowseName = browseResult.BrowseName;
                    plElement.pNode = TransformToNodeIds(browseResult.NodeId, sharedPossibleType);

                    pPlaceholderNode->addInstance(plElement);
                }
                else  {
                    LOG(WARNING) << "Could not find a possible type for :" << static_cast<std::string>(browseResult.TypeDefinition) << ". Continuing without a candidate.";
                    //LOG(WARNING) << "Pointer shows to end()";
                }
            }
        }

        void DashboardClient::subscribeValues(
			const std::shared_ptr<const ModelOpcUa::SimpleNode> pNode,
			std::map<std::shared_ptr<const ModelOpcUa::Node>, nlohmann::json> &valueMap
		)
		{
		    LOG(INFO) << "subscribeValues "   << pNode->NodeId.Uri << ";" << pNode->NodeId.Id;

            // Only Mandatory/Optional variables
			if (isMandatoryOrOptionalVariable(pNode))
			{
                subscribeValue(pNode, valueMap);
            } else {
			    LOG(INFO) << "Not subscribed " << pNode->SpecifiedBrowseName.Uri << ";" << pNode->SpecifiedBrowseName.Name;
			}

            handleSubscribeChildNodes(pNode, valueMap);
        }

        void DashboardClient::handleSubscribeChildNodes(const std::shared_ptr<const ModelOpcUa::SimpleNode> &pNode,
                                                        std::map<std::shared_ptr<const ModelOpcUa::Node>, nlohmann::json> &valueMap) {
		    LOG(INFO) << "handleSubscribeChildNodes "   << pNode->NodeId.Uri << ";" << pNode->NodeId.Id;;
            for (auto & pChildNode : pNode->ChildNodes)
            {
                switch (pChildNode->ModellingRule)
                {
                case ModelOpcUa::Mandatory:
                case ModelOpcUa::Optional:
                {
                    bool should_break = handleSubscribeChildNode(pChildNode, valueMap);
                    if (should_break) {
                        break;
                    }
                }
                case ModelOpcUa::MandatoryPlaceholder:
                case ModelOpcUa::OptionalPlaceholder:
                {
                    bool should_break = handleSubscribePlaceholderChildNode(pChildNode, valueMap);
                    if (should_break) {
                        break;
                    }
                }
                default:
                    LOG(ERROR) << "Unknown Modelling Rule." << std::endl;
                    break;
                }
            }
        }
        
        bool DashboardClient::handleSubscribeChildNode(std::shared_ptr<const ModelOpcUa::Node> pChildNode,
                                                       std::map<std::shared_ptr<const ModelOpcUa::Node>, nlohmann::json> &valueMap){
            LOG(INFO) << "handleSubscribeChildNode " <<  pChildNode->SpecifiedBrowseName.Uri << ";" <<  pChildNode->SpecifiedBrowseName.Name;

            auto pSimpleChild = std::dynamic_pointer_cast<const ModelOpcUa::SimpleNode>(pChildNode);
            if (!pSimpleChild)
            {
                LOG(ERROR) << "Simple node error, instance not a simple node." << std::endl;
                return false;
            }
            // recursive call
            subscribeValues(pSimpleChild, valueMap);
            return true;
		}
        
        // Returns if the caller should exit the loop (break;) or not
        bool DashboardClient::handleSubscribePlaceholderChildNode(std::shared_ptr<const ModelOpcUa::Node> pChildNode,
                                                                  std::map<std::shared_ptr<const ModelOpcUa::Node>, nlohmann::json> &valueMap) {
            LOG(INFO) << "handleSubscribePlaceholderChildNode " << pChildNode->SpecifiedBrowseName.Uri << ";" << pChildNode->SpecifiedBrowseName.Name;
            auto pPlaceholderChild = std::dynamic_pointer_cast<const ModelOpcUa::PlaceholderNode>(pChildNode);
            if (!pPlaceholderChild)
            {
                LOG(ERROR) << "Placeholder error, instance not a placeholder." << std::endl;
                return true;
            }

            auto placeholderElements = pPlaceholderChild->getInstances();

            for (const auto &pPlaceholderElement : placeholderElements)
            {
                // recursive call
                subscribeValues(pPlaceholderElement.pNode, valueMap);
            }
            return true;
		}

        void DashboardClient::subscribeValue(const std::shared_ptr<const ModelOpcUa::SimpleNode> &pNode,
                                             std::map<std::shared_ptr<const ModelOpcUa::Node>, nlohmann::json> &valueMap) {/**
                                             * Creates a lambda function which gets pNode as a copy and valueMap as a reference from this function,
                                             * the input parameters of the lambda function is the nlohmann::json value and the body updates the value
                                             * at position pNode with the received json value.
                                             */
            LOG(INFO) << "SubscribeValue " << pNode->SpecifiedBrowseName.Uri << ";" << pNode->SpecifiedBrowseName.Name << " | " << pNode->NodeId.Uri << ";" << pNode->NodeId.Id;

            auto callback = [pNode, &valueMap](nlohmann::json value) {
                // LOG(INFO) << "Value Update for " << pNode->SpecifiedBrowseName.Name << " | " << pNode->NodeId.Uri << ";" << pNode->NodeId.Id << " :"<< value.dump(2);
                valueMap[pNode] = value;
            };

            auto subscribedValue = m_pDashboardDataClient->Subscribe(pNode->NodeId, callback);
            m_subscribedValues.push_back(subscribedValue);
        }

        bool DashboardClient::isMandatoryOrOptionalVariable(const std::shared_ptr<const ModelOpcUa::SimpleNode> &pNode) {
            return pNode->NodeClass == ModelOpcUa::NodeClass_t::Variable
                &&  (
                        pNode->ModellingRule == ModelOpcUa::ModellingRule_t::Mandatory
                        || pNode->ModellingRule == ModelOpcUa::ModellingRule_t::Optional
                    );

        }
    }
}
