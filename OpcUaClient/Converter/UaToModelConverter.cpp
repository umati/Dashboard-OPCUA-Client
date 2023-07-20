/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2019-2021 Christian von Arnim, ISW University of Stuttgart (for umati and VDW e.V.)
 * Copyright (c) 2020 Dominik Basner, Sotec GmbH (for VDW e.V.)
 * Copyright (c) 2021 Marius Dege, basysKom GmbH
 * Copyright (c) 2023 Sebastian Friedl, FVA GmbH interop4X
 */

#include "UaToModelConverter.hpp"
#include <easylogging++.h>

namespace Umati {
namespace OpcUa {
namespace Converter {

UaToModelConverter::UaToModelConverter(const std::map<uint16_t, std::string> &idToUri) : m_idToUri(idToUri) {}

UaToModelConverter::~UaToModelConverter() = default;

std::string UaToModelConverter::getUriFromNsIndex(uint16_t nsIndex) {
  // the opc ua standard allows to remove the nsIndex for the namespace zero
  // so the converter only use this nsIndex
  if (nsIndex == 0) {
    return std::string();
  }
  // FIX_END

  auto it = m_idToUri.find(nsIndex);
  if (it == m_idToUri.end()) {
    LOG(ERROR) << "Could not find nsIndex: " << nsIndex << std::endl;
    return std::string();
  }

  return it->second;
}
}  // namespace Converter
}  // namespace OpcUa
}  // namespace Umati
