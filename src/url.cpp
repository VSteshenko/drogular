#include <drogular/url.hpp>

#include <iomanip>
#include <sstream>

namespace drogular {

std::string Url::encode(
    const std::string& value
) {
    std::ostringstream encoded;

    encoded << std::uppercase
            << std::hex
            << std::setfill('0');

    for (const unsigned char ch : value) {
        if ((ch >= 'A' && ch <= 'Z') ||
            (ch >= 'a' && ch <= 'z') ||
            (ch >= '0' && ch <= '9') ||
            ch == '-' ||
            ch == '_' ||
            ch == '.' ||
            ch == '~') {
            encoded << ch;
        } else {
            encoded << '%'
                    << std::setw(2)
                    << static_cast<int>(ch);
        }
    }

    return encoded.str();
}

} // namespace drogular