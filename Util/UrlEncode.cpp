#include "UrlEncode.hpp"
#include <sstream>
#include <set>
#include <iomanip>

namespace Umati
{
    namespace Util
    {
        const std::set<char> Whitelist = {'-', '.', '_', '~'};
        static inline bool requireEncoding(char c)
        {
            return !isalnum(c) && (Whitelist.find(c) == Whitelist.end());
        }

        std::string UrlEncode(const std::string &url)
        {
            std::stringstream ss;
            ss << std::hex;
            for (const auto &c : url)
            {
                if (requireEncoding(c))
                {
                    ss << "%" << std::uppercase
                       << std::setw(2) << std::setfill('0') << static_cast<int>(c)
                       << std::nouppercase;
                }
                else
                {
                    ss << c;
                }
            }
            return ss.str();
        }
    } // namespace Util
} // namespace Umati
