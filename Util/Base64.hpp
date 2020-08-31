//
//  base64 encoding and decoding with C++.
//  Version: 2.rc.04 (release candidate)
//


#include <string>


namespace Umati
{
    namespace Util
    {
        class StringUtils {
        public:
            static std::string base64_encode(const std::string &s, bool url);
        private:
            static std::string base64_encode(unsigned char const* bytes_to_encode, size_t in_len, bool url);
        };
    }
}

