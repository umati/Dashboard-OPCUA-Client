#include <gtest/gtest.h>
#include <UrlEncode.hpp>

namespace Umati {
    namespace Tests {
        TEST(UrlEncode, SpecialCharacters) {
            EXPECT_EQ(Umati::Util::UrlEncode(":"), "%3A");

        }
    }
}
