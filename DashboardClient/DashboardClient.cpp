#include "DashboardClient.hpp"

#include <easylogging++.h>
#include "Converter/ModelToJson.hpp"
#include "Exceptions/UmatiException.hpp"

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

		void DashboardClient::addDataSet(
			ModelOpcUa::NodeId_t startNodeId,
			std::shared_ptr<ModelOpcUa::StructureNode> pTypeDefinition,
			std::string channel)
		{
			auto pDataSetStorage = std::make_shared<DataSetStorage_t>();
			pDataSetStorage->startNodeId = startNodeId;
			pDataSetStorage->channel = channel;
			pDataSetStorage->pTypeDefinition = pTypeDefinition;
			pDataSetStorage->node = TransformToNodeIds(startNodeId, pTypeDefinition);
			subscribeValues(pDataSetStorage->node, pDataSetStorage->values);
			m_dataSets.push_back(pDataSetStorage);

		}

		void DashboardClient::Publish()
		{
			for (auto &pDataSetStorage : m_dataSets)
			{
				m_pPublisher->Publish(pDataSetStorage->channel, getJson(pDataSetStorage));
			}
		}

		std::string DashboardClient::getJson(std::shared_ptr<DataSetStorage_t> pDataSetStorage)
		{
			auto getValueCallback = [pDataSetStorage](const std::shared_ptr<const ModelOpcUa::Node> pNode) -> nlohmann::json
			{
				auto it = pDataSetStorage->values.find(pNode);
				if (it == pDataSetStorage->values.end())
				{
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
				case ModelOpcUa::ModellingRule_t::Mandatory:
				case ModelOpcUa::ModellingRule_t::Optional:
				{
					auto childNodeId = m_pDashboardDataClient->TranslateBrowsePathToNodeId(startNode, pChild->SpecifiedBrowseName);
					if (childNodeId.isNull())
					{
						LOG(INFO) << "Could not find '"
							<< static_cast<std::string>(startNode)
							<< "'->'"
							<< static_cast<std::string>(pChild->SpecifiedBrowseName)
							<< "'";
						continue;
					}
					foundChildNodes.push_back(TransformToNodeIds(childNodeId, pChild));

					break;
				}
				case ModelOpcUa::ModellingRule_t::MandatoryPlaceholder:
				case ModelOpcUa::ModellingRule_t::OptionalPlaceholder:
				{
					auto pPlaceholderChild = std::dynamic_pointer_cast<const ModelOpcUa::StructurePlaceholderNode>(pChild);
					if (!pPlaceholderChild)
					{
						LOG(ERROR) << "Placeholder error, instance not a placeholder." << std::endl;
						break;
					}
					auto placeholderNode = BrowsePlaceholder(startNode, pPlaceholderChild);
					foundChildNodes.push_back(placeholderNode);
					break;
				}
				default:
					LOG(ERROR) << "Unknown Modelling Rule." << std::endl;
					break;
				}
			}

			auto pNode = std::make_shared<ModelOpcUa::SimpleNode>(
				startNode,
				pTypeDefinition->SpecifiedTypeNodeId, /// <\TODO set
				*pTypeDefinition,
				foundChildNodes
				);
			return pNode;
		}

		std::shared_ptr<const ModelOpcUa::PlaceholderNode> DashboardClient::BrowsePlaceholder(
			ModelOpcUa::NodeId_t startNode,
			std::shared_ptr<const ModelOpcUa::StructurePlaceholderNode> pStrucPlaceholder
		)
		{

			if (!pStrucPlaceholder)
			{
				LOG(ERROR) << "Invalid Argument, pStrucPlaceholder is nullptr";
				throw std::invalid_argument("pStrucPlaceholder is nullptr.");
			}

			auto pPlaceholderNode = std::make_shared<ModelOpcUa::PlaceholderNode>(
				*pStrucPlaceholder,
				std::list<std::shared_ptr<const ModelOpcUa::Node>>{}
			);

			auto browseResults = m_pDashboardDataClient->Browse(startNode, pStrucPlaceholder->ReferenceType, pStrucPlaceholder->SpecifiedTypeNodeId);
			for (auto &browseResult : browseResults)
			{
				auto itPosType = std::find_if(
					pStrucPlaceholder->PossibleTypes.begin(),
					pStrucPlaceholder->PossibleTypes.end(),
					[browseResult](const std::shared_ptr<const ModelOpcUa::StructureNode> &posType) -> bool
				{
					/// \TODO handle subtypes
					return posType->SpecifiedTypeNodeId == browseResult.TypeDefinition;
				}
				);
				if (itPosType == pStrucPlaceholder->PossibleTypes.end())
				{
					LOG(WARNING) << "Could not find a possible type for :" << static_cast<std::string>(browseResult.TypeDefinition);
				}

				ModelOpcUa::PlaceholderElement plElement;
				plElement.BrowseName = browseResult.BrowseName;
				plElement.pNode = TransformToNodeIds(browseResult.NodeId, *itPosType);

				pPlaceholderNode->addInstance(plElement);
			}

			return pPlaceholderNode;
		}

		void DashboardClient::subscribeValues(
			const std::shared_ptr<const ModelOpcUa::SimpleNode> pNode,
			std::map<std::shared_ptr<const ModelOpcUa::Node>, nlohmann::json> &valueMap
		)
		{
			// Only Mandatory/Optional variables
			if (pNode->NodeClass == ModelOpcUa::NodeClass_t::Variable
				&& (pNode->ModellingRule == ModelOpcUa::ModellingRule_t::Mandatory
					|| pNode->ModellingRule == ModelOpcUa::ModellingRule_t::Optional
					)
				)
			{
				auto callback = [pNode, &valueMap](nlohmann::json value) {
					valueMap[pNode] = value;
				};

				m_subscribedValues.push_back(m_pDashboardDataClient->Subscribe(pNode->NodeId, callback));
			}

			for (auto & pChildNode : pNode->ChildNodes)
			{
				switch (pChildNode->ModellingRule)
				{
				case ModelOpcUa::ModellingRule_t::Mandatory:
				case ModelOpcUa::ModellingRule_t::Optional:
				{
					auto pSimpleChild = std::dynamic_pointer_cast<const ModelOpcUa::SimpleNode>(pChildNode);
					if (!pSimpleChild)
					{
						LOG(ERROR) << "Simple node error, instance not a simple node." << std::endl;
						continue;
					}
					subscribeValues(pSimpleChild, valueMap);
					break;
				}
				case ModelOpcUa::ModellingRule_t::MandatoryPlaceholder:
				case ModelOpcUa::ModellingRule_t::OptionalPlaceholder:
				{
					auto pPlaceholderChild = std::dynamic_pointer_cast<const ModelOpcUa::PlaceholderNode>(pChildNode);
					if (!pPlaceholderChild)
					{
						LOG(ERROR) << "Placeholder error, instance not a placeholder." << std::endl;
						break;
					}

					auto placeholderElements = pPlaceholderChild->getInstances();

					for (const auto &pPlayholderElement : placeholderElements)
					{
						subscribeValues(pPlayholderElement.pNode, valueMap);
					}
					break;
				}
				default:
					LOG(ERROR) << "Unknown Modelling Rule." << std::endl;
					break;
				}
			}

		}
	}
}
