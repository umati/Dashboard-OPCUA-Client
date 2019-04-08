#pragma once

#include <string>
#include <map>

#include <uaqualifiedname.h>

#include <ModelOpcUa/ModelDefinition.hpp>

#include "ModelToUaConverter.hpp"

namespace Umati {
	namespace OpcUa
	{
		namespace Converter {
			class ModelQualifiedNameToUaQualifiedName : public ModelToUaConverter
			{
			public:
				ModelQualifiedNameToUaQualifiedName(
					ModelOpcUa::QualifiedName_t modelQualifiedName,
					const std::map<std::string, uint16_t> &uriToID
				);

				UaQualifiedName getQualifiedName() {
					return m_qualifiedName;
				};
			private:
				UaQualifiedName m_qualifiedName;
			};
		}
	}
}
