#pragma once
#include <ModelOpcUa/ModelDefinition.hpp>

namespace Umati
{
	namespace Dashboard
	{
		namespace TypeDefinition
		{
            /**
            * Defines / Creates a structureNode for stacklights, holding Color, NumberInList and Status
            */
			std::shared_ptr<ModelOpcUa::StructureNode> getStacklightType(
				ModelOpcUa::QualifiedName_t qualifiedName = {}
			);
		}
	}
}
