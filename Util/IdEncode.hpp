#pragma once
#include <string>

namespace Umati {
    namespace Util {
        /// Encodes an id so it can be used as a topic part in mqtt
        std::string IdEncode(const std::string &id);
    }
}
