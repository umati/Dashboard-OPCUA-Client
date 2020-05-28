#pragma once

#include <ModelOpcUa/ModelDefinition.hpp>

namespace Umati
{
	namespace Dashboard
	{
		namespace TypeDefinition
		{
			// const std::string
			// UmatiNamespaceUri("http://www.umati.info");
            const std::string UmatiNamespaceUri("http://www.opcfoundation.org/UA/Machinery/");

            /**
            * Defines the types in the umati namespace, also see the opcua_dashboardclient/Tools/umati.xml
            */
			namespace NodeIds
			{
				const ModelOpcUa::NodeId_t ProductionJobType {UmatiNamespaceUri, "i=1031" };
				const ModelOpcUa::NodeId_t MachineStateModeType {UmatiNamespaceUri, "i=1028" };
				const ModelOpcUa::NodeId_t StateModeType {UmatiNamespaceUri, "i=1025" };
				const ModelOpcUa::NodeId_t ChannelStateModeType {UmatiNamespaceUri, "i=1018" };
				const ModelOpcUa::NodeId_t ControllerStateModeType {UmatiNamespaceUri, "i=1023" };
				const ModelOpcUa::NodeId_t SpindleStateModeType {UmatiNamespaceUri, "i=1024" };
				const ModelOpcUa::NodeId_t ToolType {UmatiNamespaceUri, "i=1056" };
				const ModelOpcUa::NodeId_t IdentificationType {UmatiNamespaceUri, "i=1021" };
				const ModelOpcUa::NodeId_t LampType {UmatiNamespaceUri, "i=1041" };
				const ModelOpcUa::NodeId_t MachineToolType {UmatiNamespaceUri, "i=1014" };
				const ModelOpcUa::NodeId_t ProductionJobListType {UmatiNamespaceUri, "i=1032" };
				const ModelOpcUa::NodeId_t ProductionStateMachineEMOType {UmatiNamespaceUri, "i=1030" };
				const ModelOpcUa::NodeId_t SoftwareComponentVersionType {UmatiNamespaceUri, "i=1022" };
				const ModelOpcUa::NodeId_t StacklightType {UmatiNamespaceUri, "i=1040" };
				const ModelOpcUa::NodeId_t ToolListType {UmatiNamespaceUri, "i=1049" };
			}
		}
	}
}
