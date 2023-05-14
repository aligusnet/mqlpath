#pragma once
#include <sstream>

namespace mqlpath {
template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& vector) {
    bool first = true;
    for (const auto& element : vector) {
        if (!first) {
            os << ", ";
        } else {
            first = false;
        }
        os << element;
    }
    return os;
}
};  // namespace mqlpath