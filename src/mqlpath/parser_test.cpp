#include "mqlpath/ast.h"
#include "mqlpath/ast_make.h"
#include "mqlpath/value.h"
#include <catch2/catch_test_macros.hpp>
#include <lexer.h>
#include <parser.h>

namespace mqlpath {
namespace {
Expression parse(const std::string& code) {
    Lexer lexer(code);
    Driver driver{};
    Parser parser{lexer, &driver};
    auto result = parser.parse();

    if (result != 0) {
        std::cout << driver.getErrors();
    }

    REQUIRE(result == 0);
    REQUIRE(!driver.getErrors().hasErrors());
    return driver.getAST();
}
};  // namespace

TEST_CASE("nothing value", "[parser]") {
    std::string code = "Nothing";
    auto expr = parse(code);
    auto expectedExpr = ast::expr(ast::nothing());
    REQUIRE(expectedExpr == expr);
}

TEST_CASE("integer value", "[parser]") {
    std::string code = "10";
    auto expr = parse(code);
    auto expectedExpr = ast::expr(10);
    REQUIRE(expectedExpr == expr);
}

TEST_CASE("boolean value", "[parser]") {
    std::string code = "true";
    auto expr = parse(code);
    auto expectedExpr = ast::expr(true);
    REQUIRE(expectedExpr == expr);
}

TEST_CASE("string value", "[parser]") {
    std::string code = "\"hello\"";
    auto expr = parse(code);
    auto expectedExpr = ast::expr("hello");
    REQUIRE(expectedExpr == expr);
}

TEST_CASE("array value", "[parser]") {
    std::string code = "[10, 14, 27]";
    auto expr = parse(code);
    auto expectedExpr = ast::expr(std::vector<int32_t>{10, 14, 27});
    REQUIRE(expectedExpr == expr);
}

TEST_CASE("empty array value", "[parser]") {
    std::string code = "[]";
    auto expr = parse(code);
    auto expectedExpr = ast::expr(std::vector<int32_t>{});
    REQUIRE(expectedExpr == expr);
}

TEST_CASE("object value", "[parser]") {
    std::string code = "{hello: 5, \"ab.cd\"  : \"da\", 5: 100}";
    auto expr = parse(code);
    Object object{{
        {"hello", ast::value(5)},
        {"ab.cd", ast::value("da")},
        {"5", ast::value(100)},
    }};
    auto expectedExpr = ast::expr(std::move(object));
    REQUIRE(expectedExpr == expr);
}

TEST_CASE("empty object value", "[parser]") {
    std::string code = "{  }";
    auto expr = parse(code);
    auto expectedExpr = ast::expr(Object{});
    REQUIRE(expectedExpr == expr);
}

TEST_CASE("EvalPath id", "[parser]") {
    std::string code = "EvalPath Id {}";
    auto expr = parse(code);
    auto expectedExpr = ast::evalPath(ast::id(), ast::expr(Object{}));
    REQUIRE(expectedExpr == expr);
}

TEST_CASE("EvalPath Const", "[parser]") {
    std::string code = "EvalPath (Const 7) {}";
    auto expr = parse(code);
    auto expectedExpr = ast::evalPath(ast::constPath(7), ast::expr(Object{}));
    REQUIRE(expectedExpr == expr);
}

TEST_CASE("EvalPath Default", "[parser]") {
    std::string code = "EvalPath Default 7 {}";
    auto expr = parse(code);
    auto expectedExpr = ast::evalPath(ast::defaultPath(7), ast::expr(Object{}));
    REQUIRE(expectedExpr == expr);
}

TEST_CASE("EvalPath Lambda", "[parser]") {
    std::string code = "EvalPath Lambda 7 {}";
    auto expr = parse(code);
    auto expectedExpr = ast::evalPath(ast::lambda(7), ast::expr(Object{}));
    REQUIRE(expectedExpr == expr);
}

TEST_CASE("EvalPath Drop", "[parser]") {
    std::string code = "EvalPath Drop \"a\", \"b\", \"c\" {}";
    auto expr = parse(code);
    auto expectedExpr = ast::evalPath(ast::drop({"a", "b", "c"}), ast::expr(Object{}));
    REQUIRE(expectedExpr == expr);
}

TEST_CASE("EvalPath Keep", "[parser]") {
    std::string code = "EvalPath Keep \"a\", \"b\", \"c\" {}";
    auto expr = parse(code);
    auto expectedExpr = ast::evalPath(ast::keep({"a", "b", "c"}), ast::expr(Object{}));
    REQUIRE(expectedExpr == expr);
}

TEST_CASE("EvalPath Obj", "[parser]") {
    std::string code = "EvalPath Obj {}";
    auto expr = parse(code);
    auto expectedExpr = ast::evalPath(ast::obj(), ast::expr(Object{}));
    REQUIRE(expectedExpr == expr);
}

TEST_CASE("EvalPath Arr", "[parser]") {
    std::string code = "EvalPath Arr {}";
    auto expr = parse(code);
    auto expectedExpr = ast::evalPath(ast::arr(), ast::expr(Object{}));
    REQUIRE(expectedExpr == expr);
}

TEST_CASE("EvalPath Field", "[parser]") {
    std::string code = "EvalPath Field a Id {}";
    auto expr = parse(code);
    auto expectedExpr = ast::evalPath(ast::field("a", ast::id()), ast::expr(Object{}));
    REQUIRE(expectedExpr == expr);
}

TEST_CASE("EvalPath Get", "[parser]") {
    std::string code = "EvalPath Get a Id {}";
    auto expr = parse(code);
    auto expectedExpr = ast::evalPath(ast::get("a", ast::id()), ast::expr(Object{}));
    REQUIRE(expectedExpr == expr);
}

TEST_CASE("EvalPath At", "[parser]") {
    std::string code = "EvalPath At 10 Id {}";
    auto expr = parse(code);
    auto expectedExpr = ast::evalPath(ast::at(10, ast::id()), ast::expr(Object{}));
    REQUIRE(expectedExpr == expr);
}

TEST_CASE("EvalPath Traverse", "[parser]") {
    std::string code = "EvalPath Traverse Id {}";
    auto expr = parse(code);
    auto expectedExpr = ast::evalPath(ast::traverse(ast::id()), ast::expr(Object{}));
    REQUIRE(expectedExpr == expr);
}
}  // namespace mqlpath
