
#include <ModelOpcUa/ModelDefinition.hpp>
#include "UmatiTypeDefinition.hpp"

namespace Umati
{
	namespace Dashboard
	{
		namespace TypeDefinition
		{

			std::shared_ptr<ModelOpcUa::StructureNode> getSoftwareComponentVersionType(ModelOpcUa::QualifiedName_t qualifiedName);

			std::shared_ptr<ModelOpcUa::StructureNode> getIdentificationType(ModelOpcUa::QualifiedName_t qualifiedName)
			{
				auto BuildYear = std::make_shared<ModelOpcUa::StructureNode>(
					ModelOpcUa::NodeClass_t::Variable,
					ModelOpcUa::ModellingRule_t::Mandatory,
					HasComponentTypeNodeId,
					BaseDataVariableTypeNodeId,
					ModelOpcUa::QualifiedName_t{ UmatiNamespaceUri, "BuildYear" }
				);

				auto LocationMachine = std::make_shared<ModelOpcUa::StructureNode>(
					ModelOpcUa::NodeClass_t::Variable,
					ModelOpcUa::ModellingRule_t::Mandatory,
					HasComponentTypeNodeId,
					BaseDataVariableTypeNodeId,
					ModelOpcUa::QualifiedName_t{ UmatiNamespaceUri, "LocationMachine" }
				);

				auto LocationPlant = std::make_shared<ModelOpcUa::StructureNode>(
					ModelOpcUa::NodeClass_t::Variable,
					ModelOpcUa::ModellingRule_t::Mandatory,
					HasComponentTypeNodeId,
					BaseDataVariableTypeNodeId,
					ModelOpcUa::QualifiedName_t{ UmatiNamespaceUri, "LocationPlant" }
				);

				auto Manufacturer = std::make_shared<ModelOpcUa::StructureNode>(
					ModelOpcUa::NodeClass_t::Variable,
					ModelOpcUa::ModellingRule_t::Mandatory,
					HasComponentTypeNodeId,
					BaseDataVariableTypeNodeId,
					ModelOpcUa::QualifiedName_t{ UmatiNamespaceUri, "Manufacturer" }
				);

				auto NameCatalog = std::make_shared<ModelOpcUa::StructureNode>(
					ModelOpcUa::NodeClass_t::Variable,
					ModelOpcUa::ModellingRule_t::Mandatory,
					HasComponentTypeNodeId,
					BaseDataVariableTypeNodeId,
					ModelOpcUa::QualifiedName_t{ UmatiNamespaceUri, "NameCatalog" }
				);

				auto NameCustom = std::make_shared<ModelOpcUa::StructureNode>(
					ModelOpcUa::NodeClass_t::Variable,
					ModelOpcUa::ModellingRule_t::Mandatory,
					HasComponentTypeNodeId,
					BaseDataVariableTypeNodeId,
					ModelOpcUa::QualifiedName_t{ UmatiNamespaceUri, "NameCustom" }
				);
				
				auto SerialNumber = std::make_shared<ModelOpcUa::StructureNode>(
					ModelOpcUa::NodeClass_t::Variable,
					ModelOpcUa::ModellingRule_t::Mandatory,
					HasComponentTypeNodeId,
					BaseDataVariableTypeNodeId,
					ModelOpcUa::QualifiedName_t{ UmatiNamespaceUri, "SerialNumber" }
				);
				
				auto SoftwareVersions = getSoftwareComponentVersionType({UmatiNamespaceUri, "SoftwareVersions"});

				auto IdentificationType = std::make_shared<ModelOpcUa::StructureNode>(
					ModelOpcUa::NodeClass_t::Object,
					ModelOpcUa::ModellingRule_t::Mandatory,
					ModelOpcUa::NodeId_t{},
					NodeIds::IdentificationType,
					qualifiedName,
					std::list<std::shared_ptr<const ModelOpcUa::StructureNode>>{
						BuildYear,
						LocationMachine,
						LocationPlant,
						Manufacturer,
						NameCatalog,
						NameCustom,
						SerialNumber,
						SoftwareVersions 
						}
					);

				return IdentificationType;
			}

			std::shared_ptr<ModelOpcUa::StructureNode> getSoftwareComponentVersionType(ModelOpcUa::QualifiedName_t qualifiedName)
			{
				auto Identifier = std::make_shared<ModelOpcUa::StructureNode>(
					ModelOpcUa::NodeClass_t::Variable,
					ModelOpcUa::ModellingRule_t::Mandatory,
					HasComponentTypeNodeId,
					PropertyTypeNodeId,
					ModelOpcUa::QualifiedName_t{ UmatiNamespaceUri, "Identifier" }
				);

				auto ComponentVersion = std::make_shared<ModelOpcUa::StructureNode>(
					ModelOpcUa::NodeClass_t::Variable,
					ModelOpcUa::ModellingRule_t::Mandatory,
					HasComponentTypeNodeId,
					BaseDataVariableTypeNodeId,
					ModelOpcUa::QualifiedName_t{ UmatiNamespaceUri, "ComponentVersion" }
				);

				auto SoftwareVersions = std::make_shared<ModelOpcUa::StructureNode>(
					ModelOpcUa::NodeClass_t::Object,
					ModelOpcUa::ModellingRule_t::Mandatory,
					HasComponentTypeNodeId,
					NodeIds::SoftwareComponentVersionType,
					qualifiedName,
					std::list<std::shared_ptr<const ModelOpcUa::StructureNode>>{
					Identifier,
					ComponentVersion
				}
				);

				return SoftwareVersions;
			}

		}
	}
}
