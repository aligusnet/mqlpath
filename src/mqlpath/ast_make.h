#pragma once

#include "mqlpath/ast.h"
#include "mqlpath/value.h"
#include <concepts>
#include <type_traits>

namespace mqlpath::ast {
template <class T>
concept scalar = std::is_signed_v<T> || std::is_floating_point_v<T> ||
    std::is_convertible_v<T, bool> || std::is_convertible_v<T, std::string>;

template <scalar T>
inline Value value(T val) {
    return Value::make<ScalarValue>(std::move(val));
}

template <scalar T>
Value value(std::vector<T> array) {
    std::vector<Value> valueArray{};
    valueArray.reserve(array.size());

    std::transform(begin(array), end(array), std::back_inserter(valueArray), [](const T& v) {
        return value(v);
    });
    return Value::make<ArrayValue>(std::move(valueArray));
}

inline Value value(Object object) {
    return Value::make<ObjectValue>(std::move(object));
}

inline Value value(Value value) {
    return value;
}

inline Value nothing() {
    return Value::make<NothingValue>();
}

template <typename T>
concept value_convertible = requires(T t) {
    { value(t) } -> std::convertible_to<Value>;
};

template <value_convertible T>
inline Expression expr(T v) {
    return Expression::make<ConstantValue>(value(std::move(v)), location());
    return expr(value(std::move(v)));
}

inline Expression evalPath(Path path, Expression expr) {
    return Expression::make<EvalPath>(std::move(path), std::move(expr), location());
}

template <value_convertible T>
inline Expression evalPath(Path path, T v) {
    return Expression::make<EvalPath>(std::move(path), expr(std::move(v)), location());
}

inline Path id() {
    return Path::make<IdPath>();
}

inline Path constPath(Expression expr) {
    return Path::make<ConstPath>(std::move(expr));
}

template <value_convertible T>
inline Path constPath(T val) {
    return constPath(expr(std::move(val)));
}

inline Path defaultPath(Expression expr) {
    return Path::make<DefaultPath>(std::move(expr));
}

template <scalar T>
inline Path defaultPath(T val) {
    return Path::make<DefaultPath>(expr(std::move(val)));
}

template <scalar T>
inline Path lambda(T val) {
    return Path::make<LambdaPath>(expr(std::move(val)));
}

inline Path drop(std::vector<std::string> fieldNames) {
    return Path::make<DropPath>(std::move(fieldNames));
}

inline Path keep(std::vector<std::string> fieldNames) {
    return Path::make<KeepPath>(std::move(fieldNames));
}

inline Path obj() {
    return Path::make<ObjPath>();
}

inline Path arr() {
    return Path::make<ArrPath>();
}

inline Path field(std::string fieldName, Path path) {
    return Path::make<FieldPath>(std::move(fieldName), std::move(path));
}

inline Path get(std::string fieldName, Path path) {
    return Path::make<GetPath>(std::move(fieldName), std::move(path));
}

inline Path at(int32_t index, Path path) {
    return Path::make<AtPath>(index, std::move(path));
}

inline Path traverse(Path path) {
    return Path::make<TraversePath>(std::move(path));
}

inline Path compose(Path left, Path right) {
    return Path::make<CompositionPath>(std::move(left), std::move(right));
}

}  // namespace mqlpath::ast