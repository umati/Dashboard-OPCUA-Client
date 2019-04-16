
#include "ToolListType.hpp"
#include "UmatiTypeDefinition.hpp"

namespace Umati
{
	namespace Dashboard
	{
		namespace TypeDefinition
		{
			std::shared_ptr<ModelOpcUa::StructureNode> getPlaceholderToolType(
				ModelOpcUa::QualifiedName_t qualifiedName = {}
			);

			std::shared_ptr<ModelOpcUa::StructureNode> getToolListType(ModelOpcUa::QualifiedName_t qualifiedName)
			{
				auto ToolListType = std::make_shared<ModelOpcUa::StructureNode>(
					ModelOpcUa::NodeClass_t::Object,
					ModelOpcUa::ModellingRule_t::Mandatory,
					ModelOpcUa::NodeId_t{},
					NodeIds::ToolListType,
					qualifiedName,
					std::list<std::shared_ptr<const ModelOpcUa::StructureNode>>{
					getPlaceholderToolType(ModelOpcUa::QualifiedName_t{ UmatiNamespaceUri, "Tools" })
				}
				);

				return ToolListType;
			}

			std::shared_ptr<ModelOpcUa::StructureNode> getPlaceholderToolType(
				ModelOpcUa::QualifiedName_t qualifiedName
			)
			{
				auto Active = std::make_shared<ModelOpcUa::StructureNode>(
					ModelOpcUa::NodeClass_t::Variable,
					ModelOpcUa::ModellingRule_t::Mandatory,
					HasComponentTypeNodeId,
					BaseDataVariableTypeNodeId,
					ModelOpcUa::QualifiedName_t{ UmatiNamespaceUri, "Active" }
				);

				auto Duplonumber = std::make_shared<ModelOpcUa::StructureNode>(
					ModelOpcUa::NodeClass_t::Variable,
					ModelOpcUa::ModellingRule_t::Mandatory,
					HasComponentTypeNodeId,
					BaseDataVariableTypeNodeId,
					ModelOpcUa::QualifiedName_t{ UmatiNamespaceUri, "Duplonumber" }
				);
				
				auto LastUsage = std::make_shared<ModelOpcUa::StructureNode>(
					ModelOpcUa::NodeClass_t::Variable,
					ModelOpcUa::ModellingRule_t::Mandatory,
					HasComponentTypeNodeId,
					BaseDataVariableTypeNodeId,
					ModelOpcUa::QualifiedName_t{ UmatiNamespaceUri, "LastUsage" }
				);
				
				auto Locked = std::make_shared<ModelOpcUa::StructureNode>(
					ModelOpcUa::NodeClass_t::Variable,
					ModelOpcUa::ModellingRule_t::Mandatory,
					HasComponentTypeNodeId,
					BaseDataVariableTypeNodeId,
					ModelOpcUa::QualifiedName_t{ UmatiNamespaceUri, "Locked" }
				);
				
				auto TypeId = std::make_shared<ModelOpcUa::StructureNode>(
					ModelOpcUa::NodeClass_t::Variable,
					ModelOpcUa::ModellingRule_t::Mandatory,
					HasComponentTypeNodeId,
					BaseDataVariableTypeNodeId,
					ModelOpcUa::QualifiedName_t{ UmatiNamespaceUri, "TypeId" }
				);
				
				auto UniqueId = std::make_shared<ModelOpcUa::StructureNode>(
					ModelOpcUa::NodeClass_t::Variable,
					ModelOpcUa::ModellingRule_t::Mandatory,
					HasComponentTypeNodeId,
					BaseDataVariableTypeNodeId,
					ModelOpcUa::QualifiedName_t{ UmatiNamespaceUri, "UniqueId" }
				);

				auto ToolType = std::make_shared<ModelOpcUa::StructureNode>(
					ModelOpcUa::NodeClass_t::Object,
					ModelOpcUa::ModellingRule_t::Mandatory,
					HasComponentTypeNodeId,
					NodeIds::ToolType,
					qualifiedName,
					std::list<std::shared_ptr<const ModelOpcUa::StructureNode>>{
					Active,
						Duplonumber,
						LastUsage,
						Locked,
						TypeId,
						UniqueId
				}
				);

				auto PlaceholderToolType = std::make_shared<ModelOpcUa::StructurePlaceholderNode>(
					ModelOpcUa::NodeClass_t::Object,
					ModelOpcUa::ModellingRule_t::MandatoryPlaceholder,
					HasComponentTypeNodeId,
					NodeIds::ToolType,
					qualifiedName,
					std::list<std::shared_ptr<const ModelOpcUa::StructureNode>> {},
					std::list<std::shared_ptr<const ModelOpcUa::StructureNode>> {
					ToolType
				}
				);
				return PlaceholderToolType;
			}
		}
	}
}
