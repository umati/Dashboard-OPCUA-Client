 /* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 * 
 * Copyright 2019-2021 (c) Christian von Arnim, ISW University of Stuttgart (for umati and VDW e.V.)
 * Copyright 2020 (c) Dominik Basner, Sotec GmbH (for VDW e.V.)
 * Copyright 2021 (c) Marius Dege, basysKom GmbH
 */

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
						UA_QualifiedName_clear(&m_qualifiedName);
				};

				UA_QualifiedName detach(){
					auto result = m_qualifiedName;
					m_qualifiedName = {0, {0, nullptr}}; //set member to 0. 
					return result;
				};

				UA_QualifiedName getQualifiedName() const {
					return m_qualifiedName;
				};
			private:
				UA_QualifiedName m_qualifiedName;
			};
		}
	}
}
