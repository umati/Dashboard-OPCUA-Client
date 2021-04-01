#include <gtest/gtest.h>
#include <IdEncode.hpp>

namespace Umati {
    namespace Tests {
        TEST(IdEncode, SpecialCharacters) {
            EXPECT_EQ(Umati::Util::IdEncode("/"), "_2F");

        }
    }
}
