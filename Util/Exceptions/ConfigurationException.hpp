/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright 2019-2023 (c) Christian von Arnim, ISW University of Stuttgart (for umati and VDW e.V.)
 * Copyright 2020 (c) Dominik Basner, Sotec GmbH (for VDW e.V.)
 */

#include <stdexcept>

namespace Umati {
namespace Util {
namespace Exception {
class ConfigurationException : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
};
}  // namespace Exception
}  // namespace Util
}  // namespace Umati
