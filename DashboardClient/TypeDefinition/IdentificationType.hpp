#pragma once
#include <ModelOpcUa/ModelDefinition.hpp>

namespace Umati
{
	namespace Dashboard
	{
		namespace TypeDefinition
		{
            /**
            * Defines / creates a structureNode containing nodes for BuildYear, LocationMachine, LocationPlant, Manufacturer, NameCatalog, NameCustom,
            * Serial Number and SoftwareVersions(containing an identifier and a component version).
            */
			std::shared_ptr<ModelOpcUa::StructureNode> getIdentificationType(
				ModelOpcUa::QualifiedName_t qualifiedName = {}
			);
		}
	}
}
