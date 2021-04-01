#include "IdEncode.hpp"
#include <sstream>
#include <set>
#include <iomanip>

namespace Umati
{
    namespace Util
    {
        // These characters are more than a usual urlencode, as mqtt allows more characters in the topic parts
        const std::set<char> Whitelist = {'-', '.', '~', ':', ';', '='};
        static inline bool requireEncoding(char c)
        {
            return !isalnum(c) && (Whitelist.find(c) == Whitelist.end());
        }

        std::string IdEncode(const std::string &id)
        {
            std::stringstream ss;
            ss << std::hex;
            for (const auto &c : id)
            {
                if (requireEncoding(c))
                {
                    ss << "_" << std::uppercase
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
