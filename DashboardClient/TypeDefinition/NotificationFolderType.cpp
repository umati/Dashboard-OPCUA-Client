#include "NotificationFolderType.hpp"
#include "UmatiTypeDefinition.hpp"

namespace Umati
{
	namespace Dashboard
	{
		namespace TypeDefinition
		{
			std::shared_ptr<ModelOpcUa::StructureNode> getPlaceholderNotificationType(
				ModelOpcUa::QualifiedName_t qualifiedName = {}
			);

			std::shared_ptr<ModelOpcUa::StructureNode> getNotificationFolderType(
				ModelOpcUa::QualifiedName_t qualifiedName
			)
			{
				auto NotificationFolderType = std::make_shared<ModelOpcUa::StructureNode>(
					ModelOpcUa::NodeClass_t::Object,
					ModelOpcUa::ModellingRule_t::Mandatory,
					ModelOpcUa::NodeId_t{},
					FolderTypeNodeId,
					qualifiedName,
					std::list<std::shared_ptr<const ModelOpcUa::StructureNode>>{
					getPlaceholderNotificationType(ModelOpcUa::QualifiedName_t{ UmatiNamespaceUri, "Notifications" })
				}
				);

				return NotificationFolderType;
			}

			std::shared_ptr<ModelOpcUa::StructureNode> getPlaceholderNotificationType(
				ModelOpcUa::QualifiedName_t qualifiedName
			)
			{
				
				auto ID = std::make_shared<ModelOpcUa::StructureNode>(
					ModelOpcUa::NodeClass_t::Variable,
					ModelOpcUa::ModellingRule_t::Mandatory,
					HasComponentTypeNodeId,
					BaseDataVariableTypeNodeId,
					ModelOpcUa::QualifiedName_t{ UmatiNamespaceUri, "ID" }
				);
				
				auto Text = std::make_shared<ModelOpcUa::StructureNode>(
					ModelOpcUa::NodeClass_t::Variable,
					ModelOpcUa::ModellingRule_t::Mandatory,
					HasComponentTypeNodeId,
					BaseDataVariableTypeNodeId,
					ModelOpcUa::QualifiedName_t{ UmatiNamespaceUri, "Text" }
				);
				
				auto Type = std::make_shared<ModelOpcUa::StructureNode>(
					ModelOpcUa::NodeClass_t::Variable,
					ModelOpcUa::ModellingRule_t::Mandatory,
					HasComponentTypeNodeId,
					BaseDataVariableTypeNodeId,
					ModelOpcUa::QualifiedName_t{ UmatiNamespaceUri, "Type" }
				);

				
				auto EMONotificationType = std::make_shared<ModelOpcUa::StructureNode>(
					ModelOpcUa::NodeClass_t::Object,
					ModelOpcUa::ModellingRule_t::Mandatory,
					HasComponentTypeNodeId,
					NodeIds::EMONotificationType,
					qualifiedName,
					std::list<std::shared_ptr<const ModelOpcUa::StructureNode>>{
				ID,
					Text,
					Type
				}
				);

				auto PlaceholderEMONotificationType = std::make_shared<ModelOpcUa::StructurePlaceholderNode>(
					ModelOpcUa::NodeClass_t::Object,
					ModelOpcUa::ModellingRule_t::MandatoryPlaceholder,
					HasComponentTypeNodeId,
					NodeIds::EMONotificationType,
					qualifiedName,
					std::list<std::shared_ptr<const ModelOpcUa::StructureNode>> {},
					std::list<std::shared_ptr<const ModelOpcUa::StructureNode>> {
					EMONotificationType
				}
				);
				return PlaceholderEMONotificationType;
			}
		}
	}
}
