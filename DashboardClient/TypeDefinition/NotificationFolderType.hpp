#pragma once
#include <ModelOpcUa/ModelDefinition.hpp>

namespace Umati
{
	namespace Dashboard
	{
		namespace TypeDefinition
		{
			std::shared_ptr<ModelOpcUa::StructureNode> getNotificationFolderType(
				ModelOpcUa::QualifiedName_t qualifiedName = {}
			);
		}
	}
}
