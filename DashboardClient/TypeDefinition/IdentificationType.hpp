#include <ModelOpcUa/ModelDefinition.hpp>

namespace Umati
{
	namespace Dashboard
	{
		namespace TypeDefinition
		{
			std::shared_ptr<ModelOpcUa::StructureNode> getIdentificationType(
				ModelOpcUa::QualifiedName_t qualifiedName = {}
			);
		}
	}
}
