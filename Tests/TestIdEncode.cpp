/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright 2021 (c) Christian von Arnim, ISW University of Stuttgart (for umati and VDW e.V.)
 */

#include <gtest/gtest.h>
#include <IdEncode.hpp>

namespace Umati {
namespace Tests {
TEST(IdEncode, SpecialCharacters) { EXPECT_EQ(Umati::Util::IdEncode("/"), "_2F"); }
}  // namespace Tests
}  // namespace Umati
