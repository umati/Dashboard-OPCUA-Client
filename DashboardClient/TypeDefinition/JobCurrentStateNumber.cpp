#include "JobCurrentStateNumber.hpp"
#include "UmatiTypeDefinition.hpp"

namespace Umati
{
	namespace Dashboard
	{
		namespace TypeDefinition
		{
			std::shared_ptr<ModelOpcUa::StructureNode> getJobCurrentStateNumber() {
				return std::make_shared<ModelOpcUa::StructureNode>(
					ModelOpcUa::NodeClass_t::Variable,
					ModelOpcUa::ModellingRule_t::Mandatory,
					HasPropertyTypeNodeId,
					PropertyTypeNodeId,
					ModelOpcUa::QualifiedName_t{ UmatiNamespaceUri, "Number" }
				);
			}
		}
	}
}
