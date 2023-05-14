#pragma once
#include "mqlpath/value.h"
#include <iosfwd>
#include <location.h>
#include <mongodb/polyvalue.h>

namespace mqlpath {

struct ConstantValue;
struct EvalPath;
using Expression = mongodb::PolyValue<ConstantValue, EvalPath>;

struct IdPath;
struct ConstPath;
struct DefaultPath;
struct LambdaPath;
struct DropPath;
struct KeepPath;
struct ObjPath;
struct ArrPath;
struct FieldPath;
struct GetPath;
struct AtPath;
struct TraversePath;
struct CompositionPath;
using Path = mongodb::PolyValue<IdPath,
                                ConstPath,
                                DefaultPath,
                                LambdaPath,
                                DropPath,
                                KeepPath,
                                ObjPath,
                                ArrPath,
                                FieldPath,
                                GetPath,
                                AtPath,
                                TraversePath,
                                CompositionPath>;

struct ConstantValue {
    ConstantValue(Value value, location location) : value(value), location(location) {}
    ConstantValue(Scalar scalar, location location)
        : value(Value::make<ScalarValue>(scalar)), location(location) {}
    ConstantValue(std::vector<Value> array, location location)
        : value(Value::make<ArrayValue>(std::move(array))), location(location) {}

    bool operator==(const ConstantValue& other) const {
        return value == other.value;
    }

    Value value;
    location location;
};

struct EvalPath {
    EvalPath(Path path, Expression expr, location location)
        : path(std::move(path)), expr(std::move(expr)), location(std::move(location)) {}

    bool operator==(const EvalPath& other) const {
        return path == other.path && expr == other.expr;
    }

    Path path;
    Expression expr;
    location location;
};

struct IdPath {
    bool operator==(const IdPath&) const = default;
};

struct ConstPath {
    explicit ConstPath(Expression expr) : expr(std::move(expr)) {}

    bool operator==(const ConstPath&) const = default;

    Expression expr;
};

struct DefaultPath {
    explicit DefaultPath(Expression expr) : expr(std::move(expr)) {}

    bool operator==(const DefaultPath&) const = default;

    Expression expr;
};

struct LambdaPath {
    explicit LambdaPath(Expression expr) : expr(std::move(expr)) {}

    bool operator==(const LambdaPath&) const = default;

    Expression expr;
};

struct DropPath {
    explicit DropPath(std::vector<std::string> fieldNames) : fieldNames(std::move(fieldNames)) {}
    bool operator==(const DropPath&) const = default;
    std::vector<std::string> fieldNames;
};

struct KeepPath {
    explicit KeepPath(std::vector<std::string> fieldNames) : fieldNames(std::move(fieldNames)) {}
    bool operator==(const KeepPath&) const = default;
    std::vector<std::string> fieldNames;
};

struct ObjPath {
    bool operator==(const ObjPath&) const = default;
};

struct ArrPath {
    bool operator==(const ArrPath&) const = default;
};

struct FieldPath {
    FieldPath(std::string fieldName, Path path)
        : fieldName(std::move(fieldName)), path(std::move(path)) {}
    bool operator==(const FieldPath&) const = default;
    std::string fieldName;
    Path path;
};

struct GetPath {
    GetPath(std::string fieldName, Path path)
        : fieldName(std::move(fieldName)), path(std::move(path)) {}
    bool operator==(const GetPath&) const = default;
    std::string fieldName;
    Path path;
};

struct AtPath {
    AtPath(int32_t index, Path path) : index(index), path(std::move(path)) {}
    bool operator==(const AtPath&) const = default;
    int32_t index;
    Path path;
};

struct TraversePath {
    TraversePath(Path path) : path(std::move(path)) {}
    bool operator==(const TraversePath&) const = default;
    Path path;
};

struct CompositionPath {
    CompositionPath(Path left, Path right) : left(std::move(left)), right(std::move(right)) {}
    bool operator==(const CompositionPath&) const = default;
    Path left;
    Path right;
};

std::ostream& operator<<(std::ostream& os, const Expression& expr);
std::ostream& operator<<(std::ostream& os, const Path& path);

}  // namespace mqlpath