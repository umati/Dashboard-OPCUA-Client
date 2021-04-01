
#include "UaNodeIdToModelNodeId.hpp"
#include <easylogging++.h>
#include "UaQualifiedNameToModelQualifiedName.hpp"

namespace Umati {
	namespace OpcUa {
		namespace Converter {

			UaQualifiedNameToModelQualifiedName::UaQualifiedNameToModelQualifiedName(
					const open62541Cpp::UA_QualifiedName &qualifiedName,
					const std::map<uint16_t, std::string> &idToUri)
					: UaToModelConverter(idToUri) {
				m_qualifiedName.Uri = getUriFromNsIndex(qualifiedName.QualifiedName->namespaceIndex);
				m_qualifiedName.Name = std::string((char *)qualifiedName.QualifiedName->name.data,qualifiedName.QualifiedName->name.length);
			}
		}
	}
}
