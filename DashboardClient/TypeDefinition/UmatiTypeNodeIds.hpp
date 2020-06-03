#pragma once

#include <ModelOpcUa/ModelDefinition.hpp>

namespace Umati
{
	namespace Dashboard
	{
		namespace TypeDefinition
		{

            /**
            * Defines the types in the umati namespace, also see the opcua_dashboardclient/Tools/umati.xml
            */
			namespace NodeIds
			{

				const ModelOpcUa::NodeId_t MachineToolIdentificationType {"http://opcfoundation.org/UA/MachineTool/", "i=1012" }; // todo read dynamically from typemap + namespace list

			}
		}
	}
}
