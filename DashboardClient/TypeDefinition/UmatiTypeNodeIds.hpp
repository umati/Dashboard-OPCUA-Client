#pragma once

#include <ModelOpcUa/ModelDefinition.hpp>

namespace Umati
{
	namespace Dashboard
	{
		namespace TypeDefinition
		{
			const std::string UmatiNamespaceUri("http://www.umati.info");
			
			namespace NodeIds
			{
				const ModelOpcUa::NodeId_t AlertType {UmatiNamespaceUri, "i=1020" };
				const ModelOpcUa::NodeId_t MachineToolAlarmConditionType {UmatiNamespaceUri, "i=1042" };
				const ModelOpcUa::NodeId_t InterruptionConditionType {UmatiNamespaceUri, "i=1060" };
				const ModelOpcUa::NodeId_t InterruptionClampingEventType {UmatiNamespaceUri, "i=1066" };
				const ModelOpcUa::NodeId_t InterruptionManualProcessStepEventType {UmatiNamespaceUri, "i=1068" };
				const ModelOpcUa::NodeId_t InterruptionMeasurementEventType {UmatiNamespaceUri, "i=1065" };
				const ModelOpcUa::NodeId_t InterruptionProcessIrregularityEventType {UmatiNamespaceUri, "i=1069" };
				const ModelOpcUa::NodeId_t InterruptionSafetyEventType {UmatiNamespaceUri, "i=1067" };
				const ModelOpcUa::NodeId_t InterruptionToolChangeEventType {UmatiNamespaceUri, "i=1064" };
				const ModelOpcUa::NodeId_t InterruptionUtilityEventType {UmatiNamespaceUri, "i=1070" };
				const ModelOpcUa::NodeId_t ProductionStateEventType {UmatiNamespaceUri, "i=1033" };
				const ModelOpcUa::NodeId_t ProductionEndEventType {UmatiNamespaceUri, "i=1034" };
				const ModelOpcUa::NodeId_t ProductionAbortEventType {UmatiNamespaceUri, "i=1045" };
				const ModelOpcUa::NodeId_t ProductionInterruptionEventType {UmatiNamespaceUri, "i=1035" };
				const ModelOpcUa::NodeId_t ProductionResetEventType {UmatiNamespaceUri, "i=1036" };
				const ModelOpcUa::NodeId_t ProductionStartEventType {UmatiNamespaceUri, "i=1044" };
				const ModelOpcUa::NodeId_t ProductionRestartEventType {UmatiNamespaceUri, "i=1001" };
				const ModelOpcUa::NodeId_t StateEventType {UmatiNamespaceUri, "i=1029" };
				const ModelOpcUa::NodeId_t EndEventType {UmatiNamespaceUri, "i=1058" };
				const ModelOpcUa::NodeId_t AbortEventType {UmatiNamespaceUri, "i=1059" };
				const ModelOpcUa::NodeId_t InterruptionEventType {UmatiNamespaceUri, "i=1017" };
				const ModelOpcUa::NodeId_t ResetEventType {UmatiNamespaceUri, "i=1062" };
				const ModelOpcUa::NodeId_t StartEventType {UmatiNamespaceUri, "i=1061" };
				const ModelOpcUa::NodeId_t RestartEventType {UmatiNamespaceUri, "i=1019" };
				const ModelOpcUa::NodeId_t BaseProductionType {UmatiNamespaceUri, "i=1027" };
				const ModelOpcUa::NodeId_t PartType {UmatiNamespaceUri, "i=1063" };
				const ModelOpcUa::NodeId_t ProductionJobType {UmatiNamespaceUri, "i=1031" };
				const ModelOpcUa::NodeId_t ProductionProgramType {UmatiNamespaceUri, "i=1072" };
				const ModelOpcUa::NodeId_t BasePrognosisType {UmatiNamespaceUri, "i=1002" };
				const ModelOpcUa::NodeId_t PrognosisListType {UmatiNamespaceUri, "i=1003" };
				const ModelOpcUa::NodeId_t PrognosisType {UmatiNamespaceUri, "i=1004" };
				const ModelOpcUa::NodeId_t MaintenancePrognosisType {UmatiNamespaceUri, "i=1010" };
				const ModelOpcUa::NodeId_t ManualActivityPrognosisType {UmatiNamespaceUri, "i=1011" };
				const ModelOpcUa::NodeId_t PartChangePrognosisType {UmatiNamespaceUri, "i=1006" };
				const ModelOpcUa::NodeId_t ProcessChangeoverPrognosisType {UmatiNamespaceUri, "i=1009" };
				const ModelOpcUa::NodeId_t ProductionJobEndPrognosisType {UmatiNamespaceUri, "i=1039" };
				const ModelOpcUa::NodeId_t ToolChangePrognosisType {UmatiNamespaceUri, "i=1005" };
				const ModelOpcUa::NodeId_t UtilityChangePrognosisType {UmatiNamespaceUri, "i=1007" };
				const ModelOpcUa::NodeId_t BaseStateModeType {UmatiNamespaceUri, "i=1015" };
				const ModelOpcUa::NodeId_t MachineStateModeType {UmatiNamespaceUri, "i=1028" };
				const ModelOpcUa::NodeId_t StateModeType {UmatiNamespaceUri, "i=1025" };
				const ModelOpcUa::NodeId_t ChannelStateModeType {UmatiNamespaceUri, "i=1018" };
				const ModelOpcUa::NodeId_t ControllerStateModeType {UmatiNamespaceUri, "i=1023" };
				const ModelOpcUa::NodeId_t SpindleStateModeType {UmatiNamespaceUri, "i=1024" };
				const ModelOpcUa::NodeId_t BaseToolType {UmatiNamespaceUri, "i=1055" };
				const ModelOpcUa::NodeId_t MultiToolType {UmatiNamespaceUri, "i=1057" };
				const ModelOpcUa::NodeId_t ToolType {UmatiNamespaceUri, "i=1056" };
				const ModelOpcUa::NodeId_t EMONotificationType {UmatiNamespaceUri, "i=1046" };
				const ModelOpcUa::NodeId_t MachineToolsFolderType {UmatiNamespaceUri, "i=1012" };
				const ModelOpcUa::NodeId_t IdentificationType {UmatiNamespaceUri, "i=1021" };
				const ModelOpcUa::NodeId_t LampType {UmatiNamespaceUri, "i=1041" };
				const ModelOpcUa::NodeId_t MachineToolType {UmatiNamespaceUri, "i=1014" };
				const ModelOpcUa::NodeId_t MonitoringType {UmatiNamespaceUri, "i=1016" };
				const ModelOpcUa::NodeId_t NotificationType {UmatiNamespaceUri, "i=1008" };
				const ModelOpcUa::NodeId_t PartsInProductionListType {UmatiNamespaceUri, "i=1037" };
				const ModelOpcUa::NodeId_t ProductionJobListType {UmatiNamespaceUri, "i=1032" };
				const ModelOpcUa::NodeId_t ProductionProgramListType {UmatiNamespaceUri, "i=1038" };
				const ModelOpcUa::NodeId_t ProductionStateMachineEMOType {UmatiNamespaceUri, "i=1030" };
				const ModelOpcUa::NodeId_t SoftwareComponentVersionType {UmatiNamespaceUri, "i=1022" };
				const ModelOpcUa::NodeId_t StacklightType {UmatiNamespaceUri, "i=1040" };
				const ModelOpcUa::NodeId_t PartStateMachineType {UmatiNamespaceUri, "i=1043" };
				const ModelOpcUa::NodeId_t StateModeListType {UmatiNamespaceUri, "i=1026" };
				const ModelOpcUa::NodeId_t ToolListType {UmatiNamespaceUri, "i=1049" };
				const ModelOpcUa::NodeId_t ToolManagementType {UmatiNamespaceUri, "i=1013" };
			}
		}
	}
}
