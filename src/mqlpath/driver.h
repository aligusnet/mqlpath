#pragma once

#include "mqlpath/ast.h"
#include "mqlpath/error.h"

namespace mqlpath {

class Driver {
public:
    void setAST(Expression ast) {
        _ast = std::move(ast);
    }

    const Expression& getAST() const {
        return _ast;
    }

    ErrorList& getErrors() {
        return _errors;
    }

private:
    Expression _ast;
    ErrorList _errors;
};
}  // namespace mqlpath