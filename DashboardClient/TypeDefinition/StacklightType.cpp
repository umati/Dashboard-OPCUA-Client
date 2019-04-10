
#include "StacklightType.hpp"
#include "UmatiTypeDefinition.hpp"

namespace Umati
{
	namespace Dashboard
	{
		namespace TypeDefinition
		{
			std::shared_ptr<ModelOpcUa::StructureNode> getPlaceholderLampType(
				ModelOpcUa::QualifiedName_t qualifiedName = {}
			);

			std::shared_ptr<ModelOpcUa::StructureNode> getStacklightType(
				ModelOpcUa::QualifiedName_t qualifiedName
			)
			{


				auto StacklightType = std::make_shared<ModelOpcUa::StructureNode>(
					ModelOpcUa::NodeClass_t::Object,
					ModelOpcUa::ModellingRule_t::Mandatory,
					ModelOpcUa::NodeId_t{},
					NodeIds::StacklightType,
					qualifiedName,
					std::list<std::shared_ptr<const ModelOpcUa::StructureNode>>{
					getPlaceholderLampType()
				}
				);

				return StacklightType;
			}

			std::shared_ptr<ModelOpcUa::StructureNode> getPlaceholderLampType(
				ModelOpcUa::QualifiedName_t qualifiedName
			)
			{
				auto Color = std::make_shared<ModelOpcUa::StructureNode>(
					ModelOpcUa::NodeClass_t::Variable,
					ModelOpcUa::ModellingRule_t::Mandatory,
					HasPropertyTypeNodeId,
					PropertyTypeNodeId,
					ModelOpcUa::QualifiedName_t{ UmatiNamespaceUri, "Color" }
				);

				auto NumberInList = std::make_shared<ModelOpcUa::StructureNode>(
					ModelOpcUa::NodeClass_t::Variable,
					ModelOpcUa::ModellingRule_t::Mandatory,
					HasPropertyTypeNodeId,
					PropertyTypeNodeId,
					ModelOpcUa::QualifiedName_t{ UmatiNamespaceUri, "NumberInList" }
				);

				auto Status = std::make_shared<ModelOpcUa::StructureNode>(
					ModelOpcUa::NodeClass_t::Variable,
					ModelOpcUa::ModellingRule_t::Mandatory,
					HasComponentTypeNodeId,
					BaseDataVariableTypeNodeId,
					ModelOpcUa::QualifiedName_t{ UmatiNamespaceUri, "Status" }
				);

				auto LampType = std::make_shared<ModelOpcUa::StructureNode>(
					ModelOpcUa::NodeClass_t::Object,
					ModelOpcUa::ModellingRule_t::Mandatory,
					HasComponentTypeNodeId,
					NodeIds::LampType,
					qualifiedName,
					std::list<std::shared_ptr<const ModelOpcUa::StructureNode>>{
					Color,
						NumberInList,
						Status
				}
				);

				auto PlaceholderLampType = std::make_shared<ModelOpcUa::StructurePlaceholderNode>(
					ModelOpcUa::NodeClass_t::Object,
					ModelOpcUa::ModellingRule_t::MandatoryPlaceholder,
					HasComponentTypeNodeId,
					NodeIds::LampType,
					qualifiedName,
					std::list<std::shared_ptr<const ModelOpcUa::StructureNode>> {},
					std::list<std::shared_ptr<const ModelOpcUa::StructureNode>> {
					LampType
				}
				);
				return PlaceholderLampType;
			}
		}
	}
}