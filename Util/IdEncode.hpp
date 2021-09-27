/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 * 
 * Copyright 2021 (c) Christian von Arnim, ISW University of Stuttgart (for umati and VDW e.V.)
 */

#pragma once
#include <string>

namespace Umati {
    namespace Util {
        /// Encodes an id so it can be used as a topic part in mqtt
        std::string IdEncode(const std::string &id);
    }
}
