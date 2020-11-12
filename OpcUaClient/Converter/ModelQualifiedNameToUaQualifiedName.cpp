
#include "ModelQualifiedNameToUaQualifiedName.hpp"

namespace Umati {
	namespace OpcUa {
		namespace Converter {
			ModelQualifiedNameToUaQualifiedName::ModelQualifiedNameToUaQualifiedName(
					const ModelOpcUa::QualifiedName_t &modelQualifiedName,
					const std::map<std::string, uint16_t> &uriToID
			) : ModelToUaConverter(uriToID) {
				m_qualifiedName.setQualifiedName(modelQualifiedName.Name.c_str(),
												 getNsIndexFromUri(modelQualifiedName.Uri));
			}
		}
	}
}