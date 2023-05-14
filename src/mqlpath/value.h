#pragma once
#include <iosfwd>
#include <mongodb/polyvalue.h>
#include <string>
#include <variant>
#include <vector>

namespace mqlpath {

using Scalar = std::variant<bool, int32_t, double, std::string>;

struct NothingValue;
struct ScalarValue;
struct ArrayValue;
struct ObjectValue;
using Value = mongodb::PolyValue<NothingValue, ScalarValue, ArrayValue, ObjectValue>;

inline bool isNothing(const Value& value) {
    return value.is<NothingValue>();
}

inline bool isArray(const Value& value) {
    return value.is<ArrayValue>();
}

inline bool isObject(const Value& value) {
    return value.is<ObjectValue>();
}


struct NothingValue {
    bool operator==(const NothingValue&) const {
        return true;
    }
};

struct ScalarValue {
    explicit ScalarValue(Scalar scalar) : scalar(scalar) {}
    bool operator==(const ScalarValue& other) const = default;
    Scalar scalar;
};

struct ArrayValue {
    explicit ArrayValue(std::vector<Value> array) : array(std::move(array)) {}
    bool operator==(const ArrayValue& other) const = default;
    std::vector<Value> array;
};

struct Object {
    struct Field {
        Field() {}

        bool operator==(const Field&) const = default;

        Field(std::string name, Value value) : name(std::move(name)), value(std::move(value)) {}
        std::string name;
        Value value;
    };

    Object() {}

    explicit Object(std::vector<Field> fields) : _fields(std::move(fields)) {}

    bool hasField(const std::string& fieldName) const {
        return get(fieldName) != nullptr;
    }

    const Value getValue(const std::string& fieldName) const {
        auto field = get(fieldName);
        if (field != nullptr) {
            return field->value;
        }

        return Value::make<NothingValue>();
    }

    const void setValue(const std::string& fieldName, Value value) {
        if (isNothing(value)) {
            dropFields({fieldName});
            return;
        }

        auto field = get(fieldName);
        if (field != nullptr) {
            field->value = std::move(value);
        } else {
            _fields.emplace_back(fieldName, std::move(value));
        }
    }

    void dropFields(const std::vector<std::string>& fieldNames);
    void keepFields(const std::vector<std::string>& fieldNames);

    bool operator==(const Object&) const = default;

private:
    Field* get(const std::string& fieldName) {
        auto pos = std::find_if(begin(_fields), end(_fields), [&fieldName](const Field& field) {
            return field.name == fieldName;
        });
        if (pos == end(_fields)) {
            return nullptr;
        }
        return &*pos;
    }

    const Field* get(const std::string& fieldName) const {
        auto pos = std::find_if(begin(_fields), end(_fields), [&fieldName](const Field& field) {
            return field.name == fieldName;
        });
        if (pos == end(_fields)) {
            return nullptr;
        }
        return &*pos;
    }

    friend std::ostream& operator<<(std::ostream& os, const Object& val);
    friend std::ostream& operator<<(std::ostream& os, const Object::Field& val);

    std::vector<Field> _fields;
};

struct ObjectValue {
    explicit ObjectValue(Object object) : object(std::move(object)) {}
    bool operator==(const ObjectValue& other) const = default;
    Object object;
};

std::ostream& operator<<(std::ostream& os, const Scalar& val);
std::ostream& operator<<(std::ostream& os, const Value& val);
}  // namespace mqlpath