#pragma once

#include <string>
#include <map>

#include <ModelOpcUa/ModelDefinition.hpp>

#include "ModelToUaConverter.hpp"

namespace Umati {
	namespace OpcUa {
		namespace Converter {
			class ModelQualifiedNameToUaQualifiedName : public ModelToUaConverter {
			public:
				ModelQualifiedNameToUaQualifiedName(
						const ModelOpcUa::QualifiedName_t &modelQualifiedName,
						const std::map<std::string, uint16_t> &uriToID
				);

				UA_QualifiedName getQualifiedName() {
					return m_qualifiedName;
				};
			private:
				UA_QualifiedName m_qualifiedName;
			};
		}
	}
}
