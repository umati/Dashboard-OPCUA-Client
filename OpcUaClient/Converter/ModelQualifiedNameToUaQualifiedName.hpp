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

				~ModelQualifiedNameToUaQualifiedName(){
					if(!detached){
						UA_QualifiedName_clear(&m_qualifiedName);
					}
				};

				UA_QualifiedName detach(){
					auto result = m_qualifiedName;
					m_qualifiedName = {0, {0, nullptr}}; //set member to 0. 
					detached = true;
					return result;
				};

				UA_QualifiedName getQualifiedName() const {
					return m_qualifiedName;
				};
			private:
				UA_QualifiedName m_qualifiedName;
				bool detached;
			};
		}
	}
}
