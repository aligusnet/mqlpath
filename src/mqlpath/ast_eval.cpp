#include "mqlpath/ast_eval.h"
#include "mqlpath/ast.h"
#include "mqlpath/value.h"

namespace mqlpath {
namespace {
struct PathEval {
    Value operator()(const Path&, const IdPath& path, Value value) {
        return value;
    }

    Value operator()(const Path&, const ConstPath& path, Value) {
        return evaluate(path.expr);
    }

    Value operator()(const Path&, const DefaultPath& path, Value value) {
        if (isNothing(value)) {
            return evaluate(path.expr);
        }
        return value;
    }

    Value operator()(const Path&, const LambdaPath& path, Value value) {
        // TODO;
        return Value::make<NothingValue>();
    }

    Value operator()(const Path&, const DropPath& path, Value value) {
        if (isObject(value)) {
            auto objectValue = value.cast<ObjectValue>();
            objectValue->object.dropFields(path.fieldNames);
        }
        return value;
    }

    Value operator()(const Path&, const KeepPath& path, Value value) {
        if (isObject(value)) {
            auto objectValue = value.cast<ObjectValue>();
            objectValue->object.keepFields(path.fieldNames);
        }
        return value;
    }

    Value operator()(const Path&, const ObjPath& path, Value value) {
        if (isObject(value)) {
            return value;
        }
        return Value::make<NothingValue>();
    }

    Value operator()(const Path&, const ArrPath& path, Value value) {
        if (isArray(value)) {
            return value;
        }
        return Value::make<NothingValue>();
    }

    Value operator()(const Path&, const FieldPath& path, Value value) {
        auto innerValue = Value::make<NothingValue>();
        Object object{};
        if (isObject(value)) {
            object = std::move(value.cast<ObjectValue>()->object);
            innerValue = object.getValue(path.fieldName);
        }
        innerValue = path.path.visit(*this, std::move(innerValue));
        if (isObject(value) || !isNothing(innerValue)) {
            object.setValue(path.fieldName, std::move(innerValue));
            return Value::make<ObjectValue>(std::move(object));
        }
        return value;
    }

    Value operator()(const Path&, const GetPath& path, Value value) {
        auto innerValue = Value::make<NothingValue>();
        if (isObject(value)) {
            auto objectValue = value.cast<ObjectValue>();
            innerValue = objectValue->object.getValue(path.fieldName);
        }
        return path.path.visit(*this, std::move(innerValue));
    }

    Value operator()(const Path&, const AtPath& path, Value value) {
        auto innerValue = Value::make<NothingValue>();
        if (isArray(value)) {
            auto arrayValue = value.cast<ArrayValue>();
            if (arrayValue->array.size() > static_cast<size_t>(path.index)) {
                innerValue = std::move(arrayValue->array[path.index]);
            }
        }
        return path.path.visit(*this, std::move(innerValue));
    }

    Value operator()(const Path& p, const TraversePath& path, Value value) {
        if (!isArray(value)) {
            return path.path.visit(*this, std::move(value));
        }

        auto arrayValue = value.cast<ArrayValue>();
        std::vector<Value> values{};
        values.reserve(arrayValue->array.size());

        for (auto&& element : arrayValue->array) {
            auto outValue = Value::make<NothingValue>();
            if (isArray(element)) {
                outValue = p.visit(*this, std::move(element));
            } else {
                outValue = path.path.visit(*this, std::move(element));
            }

            if (!isNothing(outValue)) {
                values.emplace_back(std::move(outValue));
            }
        }

        return Value::make<ArrayValue>(std::move(values));
    }

    Value operator()(const Path& p, const CompositionPath& path, Value value) {
        auto innerValue = path.left.visit(*this, std::move(value));
        return path.right.visit(*this, std::move(innerValue));
    }
};

struct ExpressionEval {
    Value operator()(const Expression&, const ConstantValue& expr) {
        return expr.value;
    }

    Value operator()(const Expression&, const EvalPath& expr) {
        auto value = expr.expr.visit(*this);
        PathEval eval{};
        return expr.path.visit(eval, value);
    }
};

}  // namespace

Value evaluate(const Expression& expr) {
    ExpressionEval eval{};
    return expr.visit(eval);
}
}  // namespace mqlpath