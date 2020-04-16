#pragma once
#include <ModelOpcUa/ModelDefinition.hpp>

namespace Umati
{
	namespace Dashboard
	{
		namespace TypeDefinition
		{
            /**
            * Defines / Creates a structureNode for the nodes Identifier, RunsCompleted, RunsPlanned, Number, CurrentState and State
            */
			std::shared_ptr<ModelOpcUa::StructureNode> getProductionJobListType(
				ModelOpcUa::QualifiedName_t qualifiedName = {}
			);
		}
	}
}
