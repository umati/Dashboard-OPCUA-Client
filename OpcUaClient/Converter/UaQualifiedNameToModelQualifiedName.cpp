
#include "UaNodeIdToModelNodeId.hpp"
#include <easylogging++.h>
#include "UaQualifiedNameToModelQualifiedName.hpp"

namespace Umati
{
	namespace OpcUa
	{
		namespace Converter {

			UaQualifiedNameToModelQualifiedName::UaQualifiedNameToModelQualifiedName(UaQualifiedName qualifiedName, const std::map<uint16_t, std::string>& idToUri)
				: UaToModelConverter(idToUri)
			{
				m_qualifiedName.Uri = getUriFromNsIndex(qualifiedName.namespaceIndex());
				m_qualifiedName.Name = UaString(qualifiedName.name()).toUtf8();
			}
		}
	}
}
