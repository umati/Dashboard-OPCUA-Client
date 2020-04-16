#pragma once
#include <ModelOpcUa/ModelDefinition.hpp>

namespace Umati
{
	namespace Dashboard
	{

		namespace TypeDefinition
		{
            /**
            * Defines / creates the structureNode the current state number of a job
            */
			std::shared_ptr<ModelOpcUa::StructureNode> getJobCurrentStateNumber();
		}
	}
}
