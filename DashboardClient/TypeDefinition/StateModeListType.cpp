
#include "StateModeListType.hpp"
#include "UmatiTypeDefinition.hpp"

namespace Umati
{
	namespace Dashboard
	{
		namespace TypeDefinition
		{
			std::shared_ptr<ModelOpcUa::StructureNode> getPlaceholderStateModeType(
				ModelOpcUa::QualifiedName_t qualifiedName = {}
			);

			std::shared_ptr<ModelOpcUa::StructureNode> getChannelStateModeType(
				ModelOpcUa::QualifiedName_t qualifiedName = {}
			);

			std::shared_ptr<ModelOpcUa::StructureNode> getControllerStateModeType(
				ModelOpcUa::QualifiedName_t qualifiedName = {}
			);

			std::shared_ptr<ModelOpcUa::StructureNode> getSpindleStateModeType(
				ModelOpcUa::QualifiedName_t qualifiedName = {}
			);

			std::shared_ptr<ModelOpcUa::StructureNode> getMachineStateModeType(
				ModelOpcUa::QualifiedName_t qualifiedName = {}
			);

			std::shared_ptr<ModelOpcUa::StructureNode> getStateModeListType(ModelOpcUa::QualifiedName_t qualifiedName)
			{
				auto StateModeListType = std::make_shared<ModelOpcUa::StructureNode>(
					ModelOpcUa::NodeClass_t::Object,
					ModelOpcUa::ModellingRule_t::Mandatory,
					ModelOpcUa::NodeId_t{},
					NodeIds::StateModeType,
					qualifiedName,
					std::list<std::shared_ptr<const ModelOpcUa::StructureNode>>{
					getMachineStateModeType(ModelOpcUa::QualifiedName_t{ UmatiNamespaceUri, "MachineStateMode" }),
						getPlaceholderStateModeType(ModelOpcUa::QualifiedName_t{ UmatiNamespaceUri, "StateMode" })
				}
				);

				return StateModeListType;
			}

			std::shared_ptr<ModelOpcUa::StructureNode> getPlaceholderStateModeType(
				ModelOpcUa::QualifiedName_t qualifiedName
			)
			{
				auto PlaceholderStateModeType = std::make_shared<ModelOpcUa::StructurePlaceholderNode>(
					ModelOpcUa::NodeClass_t::Object,
					ModelOpcUa::ModellingRule_t::MandatoryPlaceholder,
					HasComponentTypeNodeId,
					NodeIds::StateModeType,
					qualifiedName,
					std::list<std::shared_ptr<const ModelOpcUa::StructureNode>> {},
					std::list<std::shared_ptr<const ModelOpcUa::StructureNode>> {
					getChannelStateModeType(ModelOpcUa::QualifiedName_t{ UmatiNamespaceUri, "Channels" }),
						getControllerStateModeType(ModelOpcUa::QualifiedName_t{ UmatiNamespaceUri, "Controllers" }),
						getSpindleStateModeType(ModelOpcUa::QualifiedName_t{ UmatiNamespaceUri, "Spindles" })
				}
				);
				return PlaceholderStateModeType;
			}

			std::shared_ptr<ModelOpcUa::StructureNode> getChannelStateModeType(
				ModelOpcUa::QualifiedName_t qualifiedName
			)
			{

				auto ChannelState = std::make_shared<ModelOpcUa::StructureNode>(
					ModelOpcUa::NodeClass_t::Variable,
					ModelOpcUa::ModellingRule_t::Mandatory,
					HasComponentTypeNodeId,
					BaseDataVariableTypeNodeId,
					ModelOpcUa::QualifiedName_t{ UmatiNamespaceUri, "ChannelState" }
				);

				auto ControlMode = std::make_shared<ModelOpcUa::StructureNode>(
					ModelOpcUa::NodeClass_t::Variable,
					ModelOpcUa::ModellingRule_t::Mandatory,
					HasComponentTypeNodeId,
					BaseDataVariableTypeNodeId,
					ModelOpcUa::QualifiedName_t{ UmatiNamespaceUri, "ControlMode" }
				);

				auto FeedOverrideEURange = std::make_shared<ModelOpcUa::StructureNode>(
					ModelOpcUa::NodeClass_t::Variable,
					ModelOpcUa::ModellingRule_t::Mandatory,
					HasComponentTypeNodeId,
					BaseDataVariableTypeNodeId,
					ModelOpcUa::QualifiedName_t{ "", "EURange" }
				);

				auto FeedOverride = std::make_shared<ModelOpcUa::StructureNode>(
					ModelOpcUa::NodeClass_t::Variable,
					ModelOpcUa::ModellingRule_t::Mandatory,
					HasComponentTypeNodeId,
					BaseDataVariableTypeNodeId,
					ModelOpcUa::QualifiedName_t{ UmatiNamespaceUri, "FeedOverride" },
					std::list<std::shared_ptr<const ModelOpcUa::StructureNode>>{
					FeedOverrideEURange
				}
				);

				auto LeastOneAxisMoving = std::make_shared<ModelOpcUa::StructureNode>(
					ModelOpcUa::NodeClass_t::Variable,
					ModelOpcUa::ModellingRule_t::Mandatory,
					HasComponentTypeNodeId,
					BaseDataVariableTypeNodeId,
					ModelOpcUa::QualifiedName_t{ UmatiNamespaceUri, "LeastOneAxisMoving" }
				);

				auto Name = std::make_shared<ModelOpcUa::StructureNode>(
					ModelOpcUa::NodeClass_t::Variable,
					ModelOpcUa::ModellingRule_t::Mandatory,
					HasPropertyTypeNodeId,
					PropertyTypeNodeId,
					ModelOpcUa::QualifiedName_t{ UmatiNamespaceUri, "Name" }
				);

				auto RapidOverrideEURange = std::make_shared<ModelOpcUa::StructureNode>(
					ModelOpcUa::NodeClass_t::Variable,
					ModelOpcUa::ModellingRule_t::Mandatory,
					HasComponentTypeNodeId,
					BaseDataVariableTypeNodeId,
					ModelOpcUa::QualifiedName_t{ "", "EURange" }
				);

				auto RapidOverride = std::make_shared<ModelOpcUa::StructureNode>(
					ModelOpcUa::NodeClass_t::Variable,
					ModelOpcUa::ModellingRule_t::Mandatory,
					HasComponentTypeNodeId,
					BaseDataVariableTypeNodeId,
					ModelOpcUa::QualifiedName_t{ UmatiNamespaceUri, "RapidOverride" },
					std::list<std::shared_ptr<const ModelOpcUa::StructureNode>>{
					RapidOverrideEURange
				}
				);

				auto ChannelStateModeType = std::make_shared<ModelOpcUa::StructureNode>(
					ModelOpcUa::NodeClass_t::Object,
					ModelOpcUa::ModellingRule_t::Mandatory,
					HasComponentTypeNodeId,
					NodeIds::ChannelStateModeType,
					qualifiedName,
					std::list<std::shared_ptr<const ModelOpcUa::StructureNode>>{
					ChannelState,
						ControlMode,
						FeedOverride,
						LeastOneAxisMoving,
						Name,
						RapidOverride
				}
				);

				return ChannelStateModeType;
			}

			std::shared_ptr<ModelOpcUa::StructureNode> getControllerStateModeType(
				ModelOpcUa::QualifiedName_t qualifiedName
			)
			{
				auto Name = std::make_shared<ModelOpcUa::StructureNode>(
					ModelOpcUa::NodeClass_t::Variable,
					ModelOpcUa::ModellingRule_t::Mandatory,
					HasPropertyTypeNodeId,
					PropertyTypeNodeId,
					ModelOpcUa::QualifiedName_t{ UmatiNamespaceUri, "Name" }
				);

				auto ControllerState = std::make_shared<ModelOpcUa::StructureNode>(
					ModelOpcUa::NodeClass_t::Variable,
					ModelOpcUa::ModellingRule_t::Mandatory,
					HasComponentTypeNodeId,
					BaseDataVariableTypeNodeId,
					ModelOpcUa::QualifiedName_t{ UmatiNamespaceUri, "ControllerState" }
				);

				auto ControllerStateModeType = std::make_shared<ModelOpcUa::StructureNode>(
					ModelOpcUa::NodeClass_t::Object,
					ModelOpcUa::ModellingRule_t::Mandatory,
					HasComponentTypeNodeId,
					NodeIds::ControllerStateModeType,
					qualifiedName,
					std::list<std::shared_ptr<const ModelOpcUa::StructureNode>>{
					Name,
						ControllerState
				}
				);

				return ControllerStateModeType;
			}

			std::shared_ptr<ModelOpcUa::StructureNode> getMachineStateModeType(
				ModelOpcUa::QualifiedName_t qualifiedName
			)
			{

				auto SafetyMode = std::make_shared<ModelOpcUa::StructureNode>(
					ModelOpcUa::NodeClass_t::Variable,
					ModelOpcUa::ModellingRule_t::Mandatory,
					HasComponentTypeNodeId,
					BaseDataVariableTypeNodeId,
					ModelOpcUa::QualifiedName_t{ UmatiNamespaceUri, "SafetyMode" }
				);

				auto MachineStateModeType = std::make_shared<ModelOpcUa::StructureNode>(
					ModelOpcUa::NodeClass_t::Object,
					ModelOpcUa::ModellingRule_t::Mandatory,
					HasComponentTypeNodeId,
					NodeIds::MachineStateModeType,
					qualifiedName,
					std::list<std::shared_ptr<const ModelOpcUa::StructureNode>>{
					SafetyMode
				}
				);

				return MachineStateModeType;
			}

			std::shared_ptr<ModelOpcUa::StructureNode> getSpindleStateModeType(
				ModelOpcUa::QualifiedName_t qualifiedName
			)
			{

				auto Name = std::make_shared<ModelOpcUa::StructureNode>(
					ModelOpcUa::NodeClass_t::Variable,
					ModelOpcUa::ModellingRule_t::Mandatory,
					HasPropertyTypeNodeId,
					PropertyTypeNodeId,
					ModelOpcUa::QualifiedName_t{ UmatiNamespaceUri, "Name" }
				);

				auto SpindleOverrideEURange = std::make_shared<ModelOpcUa::StructureNode>(
					ModelOpcUa::NodeClass_t::Variable,
					ModelOpcUa::ModellingRule_t::Mandatory,
					HasComponentTypeNodeId,
					BaseDataVariableTypeNodeId,
					ModelOpcUa::QualifiedName_t{ "", "EURange" }
				);

				auto SpindleOverride = std::make_shared<ModelOpcUa::StructureNode>(
					ModelOpcUa::NodeClass_t::Variable,
					ModelOpcUa::ModellingRule_t::Mandatory,
					HasComponentTypeNodeId,
					BaseDataVariableTypeNodeId,
					ModelOpcUa::QualifiedName_t{ UmatiNamespaceUri, "SpindleOverride" },
					std::list<std::shared_ptr<const ModelOpcUa::StructureNode>>{
					SpindleOverrideEURange
				}
				);

				auto SpindleRotationSpeed = std::make_shared<ModelOpcUa::StructureNode>(
					ModelOpcUa::NodeClass_t::Variable,
					ModelOpcUa::ModellingRule_t::Mandatory,
					HasComponentTypeNodeId,
					BaseDataVariableTypeNodeId,
					ModelOpcUa::QualifiedName_t{ UmatiNamespaceUri, "SpindleRotationSpeed" }
				);

				auto SpindleStateModeType = std::make_shared<ModelOpcUa::StructureNode>(
					ModelOpcUa::NodeClass_t::Object,
					ModelOpcUa::ModellingRule_t::Mandatory,
					HasComponentTypeNodeId,
					NodeIds::SpindleStateModeType,
					qualifiedName,
					std::list<std::shared_ptr<const ModelOpcUa::StructureNode>>{
					Name,
						SpindleOverride,
						SpindleRotationSpeed
				}
				);

				return SpindleStateModeType;
			}

		}
	}
}
