#include "ProductionJobListType.hpp"
#include "UmatiTypeDefinition.hpp"

namespace Umati
{
	namespace Dashboard
	{
		namespace TypeDefinition
		{
			std::shared_ptr<ModelOpcUa::StructureNode> getPlaceholderProductionJobType(
				ModelOpcUa::QualifiedName_t qualifiedName = {}
			);

			std::shared_ptr<ModelOpcUa::StructureNode> getProductionJobListType(ModelOpcUa::QualifiedName_t qualifiedName)
			{
				auto ProductionJobListType = std::make_shared<ModelOpcUa::StructureNode>(
					ModelOpcUa::NodeClass_t::Object,
					ModelOpcUa::ModellingRule_t::Mandatory,
					ModelOpcUa::NodeId_t{},
					NodeIds::ProductionJobListType,
					qualifiedName,
					std::list<std::shared_ptr<const ModelOpcUa::StructureNode>>{
					getPlaceholderProductionJobType(ModelOpcUa::QualifiedName_t{ UmatiNamespaceUri, "Jobs" })
				}
				);

				return ProductionJobListType;
			}

			std::shared_ptr<ModelOpcUa::StructureNode> getPlaceholderProductionJobType(
				ModelOpcUa::QualifiedName_t qualifiedName
			)
			{
				auto Identifier = std::make_shared<ModelOpcUa::StructureNode>(
					ModelOpcUa::NodeClass_t::Variable,
					ModelOpcUa::ModellingRule_t::Mandatory,
					HasPropertyTypeNodeId,
					PropertyTypeNodeId,
					ModelOpcUa::QualifiedName_t{ UmatiNamespaceUri, "Identifier" }
				);

				auto RunsCompleted = std::make_shared<ModelOpcUa::StructureNode>(
					ModelOpcUa::NodeClass_t::Variable,
					ModelOpcUa::ModellingRule_t::Mandatory,
					HasPropertyTypeNodeId,
					PropertyTypeNodeId,
					ModelOpcUa::QualifiedName_t{ UmatiNamespaceUri, "RunsCompleted" }
				);

				auto RunsPlanned = std::make_shared<ModelOpcUa::StructureNode>(
					ModelOpcUa::NodeClass_t::Variable,
					ModelOpcUa::ModellingRule_t::Mandatory,
					HasPropertyTypeNodeId,
					PropertyTypeNodeId,
					ModelOpcUa::QualifiedName_t{ UmatiNamespaceUri, "RunsPlanned" }
				);

				auto StateCurrentStateNumber = std::make_shared<ModelOpcUa::StructureNode>(
					ModelOpcUa::NodeClass_t::Variable,
					ModelOpcUa::ModellingRule_t::Mandatory,
					HasPropertyTypeNodeId,
					PropertyTypeNodeId,
					ModelOpcUa::QualifiedName_t{ UmatiNamespaceUri, "Number" }
				);

				auto StateCurrentState = std::make_shared<ModelOpcUa::StructureNode>(
					ModelOpcUa::NodeClass_t::Variable,
					ModelOpcUa::ModellingRule_t::Mandatory,
					HasComponentTypeNodeId,
					BaseDataVariableTypeNodeId,
					ModelOpcUa::QualifiedName_t{ UmatiNamespaceUri, "CurrentState" },
					std::list<std::shared_ptr<const ModelOpcUa::StructureNode>>{
					StateCurrentStateNumber
				}
				);

				auto State = std::make_shared<ModelOpcUa::StructureNode>(
					ModelOpcUa::NodeClass_t::Object,
					ModelOpcUa::ModellingRule_t::Mandatory,
					HasComponentTypeNodeId,
					NodeIds::ProductionStateMachineEMOType,
					ModelOpcUa::QualifiedName_t{ UmatiNamespaceUri, "State" },
					std::list<std::shared_ptr<const ModelOpcUa::StructureNode>>{
					StateCurrentState
				}
				);

				auto ProductionJobType = std::make_shared<ModelOpcUa::StructureNode>(
					ModelOpcUa::NodeClass_t::Object,
					ModelOpcUa::ModellingRule_t::Mandatory,
					HasComponentTypeNodeId,
					NodeIds::ProductionJobType,
					qualifiedName,
					std::list<std::shared_ptr<const ModelOpcUa::StructureNode>>{
					Identifier,
						RunsCompleted,
						RunsPlanned,
						State
				}
				);

				auto PlaceholderProductionJobType = std::make_shared<ModelOpcUa::StructurePlaceholderNode>(
					ModelOpcUa::NodeClass_t::Object,
					ModelOpcUa::ModellingRule_t::MandatoryPlaceholder,
					HasComponentTypeNodeId,
					NodeIds::ProductionJobType,
					qualifiedName,
					std::list<std::shared_ptr<const ModelOpcUa::StructureNode>> {},
					std::list<std::shared_ptr<const ModelOpcUa::StructureNode>> {
					ProductionJobType
				}
				);
				return PlaceholderProductionJobType;

			}
		}
	}
}
