#include "mqlpath/ast.h"
#include "mqlpath/stream_utils.h"

namespace mqlpath {
namespace {
class ExpressionPrinter {
public:
    ExpressionPrinter(std::ostream& os) : _os(os) {}

    void operator()(const Expression&, const ConstantValue& expr) {
        _os << expr.value;
    }

    void operator()(const Expression&, const EvalPath& evalPath) {
        _os << "EvalPath " << evalPath.path << " " << evalPath.expr;
    }

private:
    std::ostream& _os;
};

class PathPrinter {
public:
    explicit PathPrinter(std::ostream& os) : _os(os) {}

    void operator()(const Path&, const IdPath&) {
        _os << "Id";
    }

    void operator()(const Path&, const ConstPath& path) {
        _os << "(Const " << path.expr << ")";
    }

    void operator()(const Path&, const DefaultPath& path) {
        _os << "(Default " << path.expr << ")";
    }

    void operator()(const Path&, const LambdaPath& path) {
        _os << "(Lambda " << path.expr << ")";
    }

    void operator()(const Path&, const DropPath& path) {
        _os << "(Drop " << path.fieldNames << ")";
    }

    void operator()(const Path&, const KeepPath& path) {
        _os << "(Keep " << path.fieldNames << ")";
    }

    void operator()(const Path&, const ObjPath&) {
        _os << "Obj";
    }

    void operator()(const Path&, const ArrPath&) {
        _os << "Arr";
    }

    void operator()(const Path&, const FieldPath& path) {
        _os << "(Field " << path.fieldName << " " << path.path << ")";
    }

    void operator()(const Path&, const GetPath& path) {
        _os << "(Get " << path.fieldName << " " << path.path << ")";
    }

    void operator()(const Path&, const AtPath& path) {
        _os << "(At " << path.index << " " << path.path << ")";
    }

    void operator()(const Path&, const TraversePath& path) {
        _os << "(Traverse " << path.path << ")";
    }

    void operator()(const Path&, const CompositionPath& path) {
        _os << "(" << path.left << " * " << path.right << ")";
    }

private:
    std::ostream& _os;
};

}  // namespace

std::ostream& operator<<(std::ostream& os, const Expression& expr) {
    ExpressionPrinter printer{os};
    expr.visit(printer);
    return os;
}

std::ostream& operator<<(std::ostream& os, const Path& path) {
    PathPrinter printer{os};
    path.visit(printer);
    return os;
}
}  // namespace mqlpath