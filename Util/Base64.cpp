#include <string>
#include <Base64.hpp>
/*
   base64.cpp and base64.h

   Copyright (C) 2004-2008 René Nyffenegger

   This source code is provided 'as-is', without any express or implied
   warranty. In no event will the author be held liable for any damages
   arising from the use of this software.

   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute it
   freely, subject to the following restrictions:

   1. The origin of this source code must not be misrepresented; you must not
      claim that you wrote the original source code. If you use this source code
      in a product, an acknowledgment in the product documentation would be
      appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
      misrepresented as being the original source code.

   3. This notice may not be removed or altered from any source distribution.

   René Nyffenegger rene.nyffenegger@adp-gmbh.ch

*/
namespace Umati {
    namespace Util {

        std::string StringUtils::base64_encode(const std::string &s, bool url) {
            return base64_encode(reinterpret_cast<const unsigned char *>(s.data()), s.length(), url);
        }


        std::string StringUtils::base64_encode(unsigned char const *bytes_to_encode, size_t in_len, bool url) {

            size_t len_encoded = (in_len + 2) / 3 * 4;

            unsigned char trailing_char = url ? '.' : '=';


            // Depending on the url parameter in base64_chars, one of
            // two sets of base64 characters needs to be chosen.
            // They differ in their last two characters.
            //
            const char *base64_chars[2] = {
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "abcdefghijklmnopqrstuvwxyz"
                    "0123456789"
                    "+/",

                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "abcdefghijklmnopqrstuvwxyz"
                    "0123456789"
                    "-_"};



            //
            // Choose set of base64 characters. They differ
            // for the last two positions, depending on the url
            // parameter.
            // A bool (as is the parameter url) is guaranteed
            // to evaluate to either 0 or 1 in C++ therfore,
            // the correct character set is chosen by subscripting
            // base64_chars with url.
            //
            const char *base64_chars_ = base64_chars[url];

            std::string ret;
            ret.reserve(len_encoded);

            unsigned int pos = 0;

            while (pos < in_len) {
                ret.push_back(base64_chars_[(bytes_to_encode[pos + 0] & 0xfc) >> 2]);

                if (pos + 1 < in_len) {
                    ret.push_back(base64_chars_[((bytes_to_encode[pos + 0] & 0x03) << 4) +
                                                ((bytes_to_encode[pos + 1] & 0xf0) >> 4)]);

                    if (pos + 2 < in_len) {
                        ret.push_back(base64_chars_[((bytes_to_encode[pos + 1] & 0x0f) << 2) +
                                                    ((bytes_to_encode[pos + 2] & 0xc0) >> 6)]);
                        ret.push_back(base64_chars_[bytes_to_encode[pos + 2] & 0x3f]);
                    } else {
                        ret.push_back(base64_chars_[(bytes_to_encode[pos + 1] & 0x0f) << 2]);
                        ret.push_back(trailing_char);
                    }
                } else {

                    ret.push_back(base64_chars_[(bytes_to_encode[pos + 0] & 0x03) << 4]);
                    ret.push_back(trailing_char);
                    ret.push_back(trailing_char);
                }

                pos += 3;
            }


            return ret;
        }

    }
}