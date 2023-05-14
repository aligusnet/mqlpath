#include <algorithm>
#include "mqlpath/stream_utils.h"
#include "mqlpath/value.h"

namespace mqlpath {
namespace {
template <class... Ts>
struct Visitor : Ts... {
    using Ts::operator()...;
};
template <class... Ts>
Visitor(Ts...) -> Visitor<Ts...>;

class ValuePrinter {
public:
    ValuePrinter(std::ostream& os) : _os(os) {}

    void operator()(const Value&, const NothingValue&) {
        _os << "Nothing";
    }

    void operator()(const Value&, const ScalarValue& scalar) {
        std::visit(Visitor{[&](const auto& v) { _os << v; }}, scalar.scalar);
    }

    void operator()(const Value&, const ArrayValue& array) {
        _os << "[" << array.array << "]";
    }

    void operator()(const Value&, const ObjectValue& object) {
        _os << object.object;
    }

private:
    std::ostream& _os;
};

bool contains(const std::vector<std::string>& vector, const std::string& element) {
    return std::find(begin(vector), end(vector), element) != end(vector);
}
}  // namespace

void Object::dropFields(const std::vector<std::string>& fieldNames) {
    auto newEnd = std::remove_if(begin(_fields), end(_fields), [&fieldNames](const auto& field) {
        return contains(fieldNames, field.name);
    });
    _fields.erase(newEnd, end(_fields));
}

void Object::keepFields(const std::vector<std::string>& fieldNames) {
    auto newEnd = std::remove_if(begin(_fields), end(_fields), [&fieldNames](const auto& field) {
        return !contains(fieldNames, field.name);
    });
    _fields.erase(newEnd, end(_fields));
}

std::ostream& operator<<(std::ostream& os, const Scalar& val) {
    std::visit(Visitor{[&os](const auto& v) { os << v; }}, val);
    return os;
}

std::ostream& operator<<(std::ostream& os, const Value& val) {
    ValuePrinter printer{os};
    val.visit(printer);
    return os;
}

std::ostream& operator<<(std::ostream& os, const Object& val) {
    os << "{" << val._fields << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const Object::Field& val) {
    os << val.name << ": " << val.value;
    return os;
}

}  // namespace mqlpath
