#include "DashboardClient.hpp"

#include <easylogging++.h>

namespace Umati {

	namespace Dashboard {

		DashboardClient::DashboardClient(std::shared_ptr<IDashboardDataClient> pDashboardDataClient)
			: m_pDashboardDataClient(pDashboardDataClient)
		{
		}

		void DashboardClient::UseDataFrom(ModelOpcUa::NodeId_t startNodeId, std::shared_ptr<ModelOpcUa::StructureNode> pTypeDefinition)
		{
			TransformToNodeIds(pTypeDefinition, startNodeId);
		}

		std::shared_ptr<const ModelOpcUa::SimpleNode> DashboardClient::TransformToNodeIds(
			const std::shared_ptr<const ModelOpcUa::StructureNode> &pTypeDefinition,
			ModelOpcUa::NodeId_t startNode
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
						continue;
					}
					foundChildNodes.push_back(TransformToNodeIds(pChild, childNodeId));

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
					auto childNodes = BrowsePlaceholder(startNode, pPlaceholderChild);
					std::cout << "Placeholder not supported." << std::endl;
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

		std::shared_ptr<const ModelOpcUa::PlaceholderNode> DashboardClient::BrowsePlaceholder(ModelOpcUa::NodeId_t startNode, std::shared_ptr<const ModelOpcUa::StructurePlaceholderNode> pStrucPlaceholder)
		{

			if (pStrucPlaceholder)
			{
				return nullptr;
			}

			//pStrucPlaceholder->PossibleTypes

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
				plElement.pNode = TransformToNodeIds(*itPosType, browseResult.NodeId);

				pPlaceholderNode->addInstance(plElement);
			}

			return pPlaceholderNode;
		}
	}
}
