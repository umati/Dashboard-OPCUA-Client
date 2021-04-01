
#include "ModelQualifiedNameToUaQualifiedName.hpp"

namespace Umati {
	namespace OpcUa {
		namespace Converter {
			ModelQualifiedNameToUaQualifiedName::ModelQualifiedNameToUaQualifiedName(
					const ModelOpcUa::QualifiedName_t &modelQualifiedName,
					const std::map<std::string, uint16_t> &uriToID
			) : ModelToUaConverter(uriToID) {
				m_qualifiedName.name.length = modelQualifiedName.Name.length();
				m_qualifiedName.name.data = (UA_Byte *)modelQualifiedName.Name.c_str();
				m_qualifiedName.namespaceIndex = getNsIndexFromUri(modelQualifiedName.Uri);
			}
		}
	}
}