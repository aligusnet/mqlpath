#include "mqlpath/ast_eval.h"
#include "mqlpath/ast_make.h"
#include "mqlpath/value.h"
#include <catch2/catch_test_macros.hpp>

namespace mqlpath {
TEST_CASE("object value", "[eval]") {
    Value value = ast::value(Object{{
        {"hello", ast::value(5)},
        {"ab.cd", ast::value("da")},
        {"5", ast::value(100)},
    }});
    auto expr = ast::expr(value);
    auto actualValue = evaluate(expr);
    REQUIRE(value == actualValue);
}

// EvalPath Id

TEST_CASE("EvalPath Id 5 => 5", "[eval]") {
    auto expr = ast::evalPath(ast::id(), 5);
    auto actualValue = evaluate(expr);
    REQUIRE(ast::value(5) == actualValue);
}

TEST_CASE("EvalPath Id {foo:2} => {foo:2}", "[eval]") {
    Object object{{{"foo", ast::value(2)}}};
    auto expr = ast::evalPath(ast::id(), object);
    auto actualValue = evaluate(expr);
    REQUIRE(ast::value(object) == actualValue);
}

TEST_CASE("EvalPath Id Nothing => Nothing", "[eval]") {
    auto expr = ast::evalPath(ast::id(), ast::nothing());
    auto actualValue = evaluate(expr);
    REQUIRE(ast::nothing() == actualValue);
}

// EvalPath Const

TEST_CASE("EvalPath (Const 5) 7 => 5", "[eval]") {
    auto expr = ast::evalPath(ast::constPath(5), ast::value(7));
    auto actualValue = evaluate(expr);
    REQUIRE(ast::value(5) == actualValue);
}

TEST_CASE("EvalPath (Const 5) {foo:2} => 5", "[eval]") {
    Object object{{{"foo", ast::value(2)}}};
    auto expr = ast::evalPath(ast::constPath(5), ast::value(object));
    auto actualValue = evaluate(expr);
    REQUIRE(ast::value(5) == actualValue);
}

TEST_CASE("EvalPath (Const 5) Nothing => 5", "[eval]") {
    auto expr = ast::evalPath(ast::constPath(5), ast::nothing());
    auto actualValue = evaluate(expr);
    REQUIRE(ast::value(5) == actualValue);
}

TEST_CASE("EvalPath (Const {a:1}) 7 => {a:1}", "[eval]") {
    Object a1{{{"a", ast::value(1)}}};
    auto expr = ast::evalPath(ast::constPath(a1), ast::value(7));
    auto actualValue = evaluate(expr);
    REQUIRE(ast::value(a1) == actualValue);
}

TEST_CASE("EvalPath (Const {a:1}) {foo:2} => {a:1}", "[eval]") {
    Object a1{{{"a", ast::value(1)}}};
    Object foo2{{{"foo", ast::value(2)}}};
    auto expr = ast::evalPath(ast::constPath(a1), ast::value(foo2));
    auto actualValue = evaluate(expr);
    REQUIRE(ast::value(a1) == actualValue);
}

TEST_CASE("EvalPath (Const {a:1}) Nothing => {a:1}", "[eval]") {
    Object a1{{{"a", ast::value(1)}}};
    auto expr = ast::evalPath(ast::constPath(a1), ast::nothing());
    auto actualValue = evaluate(expr);
    REQUIRE(ast::value(a1) == actualValue);
}

TEST_CASE("EvalPath (Const Nothing) 7 => Nothing", "[eval]") {
    auto expr = ast::evalPath(ast::constPath(ast::nothing()), ast::value(7));
    auto actualValue = evaluate(expr);
    REQUIRE(ast::nothing() == actualValue);
}

// EvalPath Default

TEST_CASE("EvalPath (Default 4) Nothing => 4", "[eval]") {
    auto expr = ast::evalPath(ast::defaultPath(4), ast::nothing());
    auto actualValue = evaluate(expr);
    REQUIRE(ast::value(4) == actualValue);
}

TEST_CASE("EvalPath (Default 4) 7 => 7", "[eval]") {
    auto expr = ast::evalPath(ast::defaultPath(4), ast::value(7));
    auto actualValue = evaluate(expr);
    REQUIRE(ast::value(7) == actualValue);
}

TEST_CASE("EvalPath (Default 4) {a:5} => {a:5}", "[eval]") {
    Object a5{{{"a", ast::value(5)}}};
    auto expr = ast::evalPath(ast::defaultPath(4), ast::value(a5));
    auto actualValue = evaluate(expr);
    REQUIRE(ast::value(a5) == actualValue);
}

// EvalPath Drop

TEST_CASE("EvalPath (Drop a) 7 => 7", "[eval]") {
    auto expr = ast::evalPath(ast::drop({"a"}), ast::value(7));
    auto actualValue = evaluate(expr);
    REQUIRE(ast::value(7) == actualValue);
}

TEST_CASE("EvalPath (Drop a) {foo:5} => {foo:5}", "[eval]") {
    Object object{{{"foo", ast::value(5)}}};
    auto expr = ast::evalPath(ast::drop({"a"}), ast::value(object));
    auto actualValue = evaluate(expr);
    REQUIRE(ast::value(object) == actualValue);
}

TEST_CASE("EvalPath (Drop a) {a:7} => {}", "[eval]") {
    Object object{{{"a", ast::value(7)}}};
    Object expected{};
    auto expr = ast::evalPath(ast::drop({"a"}), ast::value(object));
    auto actualValue = evaluate(expr);
    REQUIRE(ast::value(expected) == actualValue);
}

TEST_CASE("EvalPath (Drop a) {a:7, b:9} => {b:9}", "[eval]") {
    Object object{{
        {"a", ast::value(7)},
        {"b", ast::value(9)},
    }};
    Object expected{{
        {"b", ast::value(9)},
    }};
    auto expr = ast::evalPath(ast::drop({"a"}), ast::value(object));
    auto actualValue = evaluate(expr);
    REQUIRE(ast::value(expected) == actualValue);
}

TEST_CASE("EvalPath (Drop a, b) {a:7, b:9} => {}", "[eval]") {
    Object object{{
        {"a", ast::value(7)},
        {"b", ast::value(9)},
    }};
    Object expected{};
    auto expr = ast::evalPath(ast::drop({"a", "b"}), ast::value(object));
    auto actualValue = evaluate(expr);
    REQUIRE(ast::value(expected) == actualValue);
}

TEST_CASE("EvalPath (Drop a) Nothing => Nothing", "[eval]") {
    auto expr = ast::evalPath(ast::drop({"a"}), ast::nothing());
    auto actualValue = evaluate(expr);
    REQUIRE(ast::nothing() == actualValue);
}

// EvalPath Keep

TEST_CASE("EvalPath (Keep a) 5 => 5", "[eval]") {
    auto expr = ast::evalPath(ast::keep({"a"}), ast::value(5));
    auto actualValue = evaluate(expr);
    REQUIRE(ast::value(5) == actualValue);
}

TEST_CASE("EvalPath (Keep a) {foo: 7} => {}", "[eval]") {
    Object object{{{"foo", ast::value(7)}}};
    Object expected{};
    auto expr = ast::evalPath(ast::keep({"a"}), ast::value(object));
    auto actualValue = evaluate(expr);
    REQUIRE(ast::value(expected) == actualValue);
}

TEST_CASE("EvalPath (Keep a) {a:7, b:9} => {a:7}", "[eval]") {
    Object object{{
        {"a", ast::value(7)},
        {"b", ast::value(9)},
    }};
    Object expected{{
        {"a", ast::value(7)},
    }};
    auto expr = ast::evalPath(ast::keep({"a"}), ast::value(object));
    auto actualValue = evaluate(expr);
    REQUIRE(ast::value(expected) == actualValue);
}

TEST_CASE("EvalPath (Keep a, b) {a:7, b:9} => {a:7, b:9}", "[eval]") {
    Object object{{
        {"a", ast::value(7)},
        {"b", ast::value(9)},
    }};
    Object expected{{
        {"a", ast::value(7)},
        {"b", ast::value(9)},
    }};
    auto expr = ast::evalPath(ast::keep({"a", "b"}), ast::value(object));
    auto actualValue = evaluate(expr);
    REQUIRE(ast::value(expected) == actualValue);
}

TEST_CASE("EvalPath (Keep a) Nothing => Nothing", "[eval]") {
    auto expr = ast::evalPath(ast::keep({"a"}), ast::nothing());
    auto actualValue = evaluate(expr);
    REQUIRE(ast::nothing() == actualValue);
}

// ElemPath Obj

TEST_CASE("EvalPath Obj 5 => Nothing", "[eval]") {
    auto expr = ast::evalPath(ast::obj(), ast::value(5));
    auto actualValue = evaluate(expr);
    REQUIRE(ast::nothing() == actualValue);
}

TEST_CASE("EvalPath Obj {a: 7} => {a: 7}", "[eval]") {
    Object object{{
        {"a", ast::value(7)},
    }};
    auto expr = ast::evalPath(ast::obj(), ast::value(object));
    auto actualValue = evaluate(expr);
    REQUIRE(ast::value(object) == actualValue);
}

// EvalPath Arr

TEST_CASE("EvalPath Arr 5 => Nothing", "[eval]") {
    auto expr = ast::evalPath(ast::arr(), ast::value(5));
    auto actualValue = evaluate(expr);
    REQUIRE(ast::nothing() == actualValue);
}

TEST_CASE("EvalPath Arr [1,2,3] => [1,2,3]", "[eval]") {
    std::vector<int32_t> array{1, 2, 3};
    auto expr = ast::evalPath(ast::arr(), ast::value(array));
    auto actualValue = evaluate(expr);
    REQUIRE(ast::value(array) == actualValue);
}

// EvalPath Field

TEST_CASE("EvalPath (Field a Id) Nothing => Nothing", "[eval]") {
    auto expr = ast::evalPath(ast::field("a", ast::id()), ast::nothing());
    auto actualValue = evaluate(expr);
    REQUIRE(ast::nothing() == actualValue);
}

TEST_CASE("EvalPath (Field a Id) 5 => 5", "[eval]") {
    auto expr = ast::evalPath(ast::field("a", ast::id()), ast::value(5));
    auto actualValue = evaluate(expr);
    REQUIRE(ast::value(5) == actualValue);
}

TEST_CASE("EvalPath (Field a Id) {b:7} => {b:7}", "[eval]") {
    Object object{{
        {"b", ast::value(7)},
    }};
    auto expr = ast::evalPath(ast::field("a", ast::id()), ast::value(object));
    auto actualValue = evaluate(expr);
    REQUIRE(ast::value(object) == actualValue);
}

TEST_CASE("EvalPath (Field a Id) {a:5} => {a:5}", "[eval]") {
    Object object{{
        {"a", ast::value(5)},
    }};
    auto expr = ast::evalPath(ast::field("a", ast::id()), ast::value(object));
    auto actualValue = evaluate(expr);
    REQUIRE(ast::value(object) == actualValue);
}

TEST_CASE("EvalPath (Field a Const 7) 5 => {a:7}", "[eval]") {
    Object expected{{
        {"a", ast::value(7)},
    }};
    auto expr = ast::evalPath(ast::field("a", ast::constPath(7)), ast::value(5));
    auto actualValue = evaluate(expr);
    REQUIRE(ast::value(expected) == actualValue);
}

TEST_CASE("EvalPath (Field a Const 7) Nothing => {a:7}", "[eval]") {
    Object expected{{
        {"a", ast::value(7)},
    }};
    auto expr = ast::evalPath(ast::field("a", ast::constPath(7)), ast::nothing());
    auto actualValue = evaluate(expr);
    REQUIRE(ast::value(expected) == actualValue);
}

TEST_CASE("EvalPath (Field a Const Nothing) 5 => 5", "[eval]") {
    auto expr = ast::evalPath(ast::field("a", ast::constPath(ast::nothing())), ast::value(5));
    auto actualValue = evaluate(expr);
    REQUIRE(ast::value(5) == actualValue);
}

TEST_CASE("EvalPath (Field a Const 7) {a:5} => {a:7}", "[eval]") {
    Object object{{
        {"a", ast::value(5)},
    }};
    Object expected{{
        {"a", ast::value(7)},
    }};
    auto expr = ast::evalPath(ast::field("a", ast::constPath(7)), ast::value(object));
    auto actualValue = evaluate(expr);
    REQUIRE(ast::value(expected) == actualValue);
}

TEST_CASE("EvalPath (Field a Const 7) {b:7} => {b:7, a:7}", "[eval]") {
    Object object{{
        {"b", ast::value(7)},
    }};
    Object expected{{
        {"b", ast::value(7)},
        {"a", ast::value(7)},
    }};
    auto expr = ast::evalPath(ast::field("a", ast::constPath(7)), ast::value(object));
    auto actualValue = evaluate(expr);
    REQUIRE(ast::value(expected) == actualValue);
}

// EvalPath Get

TEST_CASE("EvalPath (Get a Id) 5 => Nothing", "[eval]") {
    auto expr = ast::evalPath(ast::get("a", ast::id()), ast::value(5));
    auto actualValue = evaluate(expr);
    REQUIRE(ast::nothing() == actualValue);
}

TEST_CASE("EvalPath (Get a Id) {b:7} => Nothing", "[eval]") {
    Object object{{
        {"b", ast::value(7)},
    }};
    auto expr = ast::evalPath(ast::get("a", ast::id()), ast::value(object));
    auto actualValue = evaluate(expr);
    REQUIRE(ast::nothing() == actualValue);
}

TEST_CASE("EvalPath (Get a Id) {a:5} => 5", "[eval]") {
    Object object{{
        {"a", ast::value(5)},
    }};
    auto expr = ast::evalPath(ast::get("a", ast::id()), ast::value(object));
    auto actualValue = evaluate(expr);
    REQUIRE(ast::value(5) == actualValue);
}

TEST_CASE("EvalPath (Get a Const 7) 5 => 7", "[eval]") {
    auto expr = ast::evalPath(ast::get("a", ast::constPath(7)), ast::value(5));
    auto actualValue = evaluate(expr);
    REQUIRE(ast::value(7) == actualValue);
}

TEST_CASE("EvalPath (Get a Const 7) {a:5} => 7", "[eval]") {
    Object object{{
        {"a", ast::value(5)},
    }};
    auto expr = ast::evalPath(ast::get("a", ast::constPath(7)), ast::value(object));
    auto actualValue = evaluate(expr);
    REQUIRE(ast::value(7) == actualValue);
}

TEST_CASE("EvalPath (Get a Const 7) {b:7} => 7", "[eval]") {
    Object object{{
        {"b", ast::value(7)},
    }};
    auto expr = ast::evalPath(ast::get("a", ast::constPath(7)), ast::value(object));
    auto actualValue = evaluate(expr);
    REQUIRE(ast::value(7) == actualValue);
}

// EvalPath At

TEST_CASE("EvalPath (At 2 Id) 5 => Nothing", "[eval]") {
    auto expr = ast::evalPath(ast::at(2, ast::id()), ast::value(5));
    auto actualValue = evaluate(expr);
    REQUIRE(ast::nothing() == actualValue);
}

TEST_CASE("EvalPath (At 2 Id) [0,1] => Nothing", "[eval]") {
    auto expr = ast::evalPath(ast::at(2, ast::id()), ast::value(std::vector<int32_t>{0, 1}));
    auto actualValue = evaluate(expr);
    REQUIRE(ast::nothing() == actualValue);
}

TEST_CASE("EvalPath (At 2 Id) [0,1,2] => 2", "[eval]") {
    auto expr = ast::evalPath(ast::at(2, ast::id()), ast::value(std::vector<int32_t>{0, 1, 2}));
    auto actualValue = evaluate(expr);
    REQUIRE(ast::value(2) == actualValue);
}

TEST_CASE("EvalPath (At 2 Default foo) 5 => foo", "[eval]") {
    auto expr = ast::evalPath(ast::at(2, ast::defaultPath("foo")), ast::value(5));
    auto actualValue = evaluate(expr);
    REQUIRE(ast::value("foo") == actualValue);
}

TEST_CASE("EvalPath (At 2 Default foo) [0,1] => foo", "[eval]") {
    auto expr =
        ast::evalPath(ast::at(2, ast::defaultPath("foo")), ast::value(std::vector<int32_t>{0, 1}));
    auto actualValue = evaluate(expr);
    REQUIRE(ast::value("foo") == actualValue);
}

TEST_CASE("EvalPath (At 2 Default foo) [0,1,2] => 2", "[eval]") {
    auto expr = ast::evalPath(ast::at(2, ast::defaultPath("foo")),
                              ast::value(std::vector<int32_t>{0, 1, 2}));
    auto actualValue = evaluate(expr);
    REQUIRE(ast::value(2) == actualValue);
}

// EvalPath Traverse

TEST_CASE("EvalPath (Traverse Id) 5 => 5", "[eval]") {
    auto expr = ast::evalPath(ast::traverse(ast::id()), ast::value(5));
    auto actualValue = evaluate(expr);
    REQUIRE(ast::value(5) == actualValue);
}

TEST_CASE("EvalPath (Traverse Id) [1,2,3] => [1,2,3]", "[eval]") {
    auto expr = ast::evalPath(ast::traverse(ast::id()), ast::value(std::vector<int32_t>{1, 2, 3}));
    auto actualValue = evaluate(expr);
    REQUIRE(ast::value(std::vector<int32_t>{1, 2, 3}) == actualValue);
}

TEST_CASE("EvalPath (Traverse Const 7) [1,2,3] => [7,7,7]", "[eval]") {
    auto expr =
        ast::evalPath(ast::traverse(ast::constPath(7)), ast::value(std::vector<int32_t>{1, 2, 3}));
    auto actualValue = evaluate(expr);
    REQUIRE(ast::value(std::vector<int32_t>{7, 7, 7}) == actualValue);
}

TEST_CASE("EvalPath (Traverse Const 7) [[1,2,3], 4] => [[7,7,7], 7]", "[eval]") {
    auto inputValue = Value::make<ArrayValue>(std::vector<Value>{
        ast::value(std::vector<int32_t>{1, 2, 3}),
        ast::value(4),
    });
    auto expectedValue = Value::make<ArrayValue>(std::vector<Value>{
        ast::value(std::vector<int32_t>{7, 7, 7}),
        ast::value(7),
    });
    auto expr = ast::evalPath(ast::traverse(ast::constPath(7)), inputValue);
    auto actualValue = evaluate(expr);
    REQUIRE(expectedValue == actualValue);
}

// EvalPath Composition

TEST_CASE("EvalPath (Field “a” Const 7) * (Field “b” Const 9) Nothing => {a:7, b:9}", "[eval]") {
    Object expected{{
        {"a", ast::value(7)},
        {"b", ast::value(9)},
    }};
    auto expr = ast::evalPath(
        ast::compose(ast::field("a", ast::constPath(7)), ast::field("b", ast::constPath(9))),
        ast::nothing());

    auto actualValue = evaluate(expr);
    REQUIRE(ast::value(expected) == actualValue);
}

TEST_CASE("EvalPath (Field “b” Const 9) * (Field “a” Const 7) Nothing => {b:9, a:7}", "[eval]") {
    Object expected{{
        {"b", ast::value(9)},
        {"a", ast::value(7)},
    }};
    auto expr = ast::evalPath(
        ast::compose(ast::field("b", ast::constPath(9)), ast::field("a", ast::constPath(7))),
        ast::nothing());

    auto actualValue = evaluate(expr);
    REQUIRE(ast::value(expected) == actualValue);
}

TEST_CASE("EvalPath (Field “a” Const 7) * (Field “b” Const 9) {a:1, b:2, c:3} => {a:7, b:9, c:3}",
          "[eval]") {
    Object object{{
        {"a", ast::value(1)},
        {"b", ast::value(2)},
        {"c", ast::value(3)},
    }};
    Object expected{{
        {"a", ast::value(7)},
        {"b", ast::value(9)},
        {"c", ast::value(3)},
    }};
    auto expr = ast::evalPath(
        ast::compose(ast::field("a", ast::constPath(7)), ast::field("b", ast::constPath(9))),
        ast::value(object));

    auto actualValue = evaluate(expr);
    REQUIRE(ast::value(expected) == actualValue);
}

TEST_CASE(
    "EvalPath (Field “a” Const 7) * (Field “b” Const 9) * (Keep “a”,”b”) {a:1, b:2, c:3} => {a:7, "
    "b:9}",
    "[eval]") {
    Object object{{
        {"a", ast::value(1)},
        {"b", ast::value(2)},
        {"c", ast::value(3)},
    }};
    Object expected{{
        {"a", ast::value(7)},
        {"b", ast::value(9)},
    }};
    auto firstCompose =
        ast::compose(ast::field("a", ast::constPath(7)), ast::field("b", ast::constPath(9)));
    auto secondCompose = ast::compose(std::move(firstCompose), ast::keep({"a", "b"}));
    auto expr = ast::evalPath(std::move(secondCompose), ast::value(object));

    auto actualValue = evaluate(expr);
    REQUIRE(ast::value(expected) == actualValue);
}
}  // namespace mqlpath