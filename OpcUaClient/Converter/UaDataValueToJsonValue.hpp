/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright 2019-2021 (c) Christian von Arnim, ISW University of Stuttgart (for umati and VDW e.V.)
 * Copyright 2020 (c) Dominik Basner, Sotec GmbH (for VDW e.V.)
 * Copyright 2021 (c) Marius Dege, basysKom GmbH
 */

#pragma once

#include <nlohmann/json.hpp>
#include <open62541/client.h>

namespace Umati {
namespace OpcUa {
namespace Converter {
class UaDataValueToJsonValue {
 public:
  explicit UaDataValueToJsonValue(const UA_DataValue &dataValue, bool serializeStatusInformation = false);

  nlohmann::json getValue() { return m_value; };

 protected:
  void setValueFromScalarVariant(UA_Variant &variant, nlohmann::json *jsonValue, bool serializeStatusInformation);

  void setValueFromArrayVariant(UA_Variant &variant, nlohmann::json *jsonValue, bool serializeStatusInformation);

  template <typename T>
  void getValueFromDataValueArray(const UA_Variant *variant, UA_UInt32 dimensionNumber, nlohmann::json *j, T *variantData, bool serializeNodeInformation);

  void setValueFromDataValue(const UA_DataValue &dataValue, bool serializeStatusInformation = false);

  void setStatusCodeFromDataValue(const UA_DataValue &dataValue);

  nlohmann::json m_value;
};
}  // namespace Converter
}  // namespace OpcUa
}  // namespace Umati
