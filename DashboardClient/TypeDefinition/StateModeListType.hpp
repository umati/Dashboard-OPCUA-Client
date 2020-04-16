#pragma once
#include <ModelOpcUa/ModelDefinition.hpp>

namespace Umati
{
	namespace Dashboard
	{
		namespace TypeDefinition
		{
            /**
            * Defines / Creates a structureNode with nodes holding the stateModelList. Contains information like
            * - MachineStateMode
            *   - SafetyMode
            * - StateMode
            *   - Channels
            *     - Channel State
            *     - ControlMode
            *     - EURange
            *     - FeedOverride
            *     - LeastOneAxisMoving
            *     - Name
            *     - RapidOverride
            *   - Controllers
            *     - Name
            *     - ControllerState
            *   - Spindles
            *     - Name
            *     - OverrideEURange
            *     - SpindleOverride
            *     - SpindleRotationSpeed
            */
			std::shared_ptr<ModelOpcUa::StructureNode> getStateModeListType(
				ModelOpcUa::QualifiedName_t qualifiedName = {}
			);
		}
	}
}
