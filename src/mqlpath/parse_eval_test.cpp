#include "mqlpath/ast_eval.h"
#include <catch2/catch_message.hpp>
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <lexer.h>
#include <parser.h>
#include <reflex/stdmatcher.h>

namespace mqlpath {
namespace {
Expression parse(const std::string& code) {
    INFO(code);
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

Value evaluate(const std::string& code) {
    auto expr = parse(code);
    return evaluate(expr);
}

struct TestCase {
    std::string expression;
    std::string expectedDocument;
};

std::vector<std::string> testCases{
    // EvalPath Id
    R"_(EvalPath Id 5 => 5)_",
    R"_(EvalPath Id {foo:2} => {foo:2})_",
    R"_(EvalPath Id Nothing => Nothing)_",

    // EvalPath Const
    R"_(EvalPath (Const 5) 7 => 5)_",
    R"_(EvalPath (Const 5) {foo:2} => 5)_",
    R"_(EvalPath (Const 5) Nothing => 5)_",
    R"_(EvalPath (Const {a:1}) 7 => {a:1})_",
    R"_(EvalPath (Const {a:1}) {f:2} => {a:1})_",
    R"_(EvalPath (Const {a:1}) Nothing => {a:1})_",
    R"_(EvalPath (Const Nothing) 7 => Nothing)_",

    // EvalPath Default
    R"_(EvalPath (Default 4) Nothing => 4)_",
    R"_(EvalPath (Default 4) 7 => 7)_",
    R"_(EvalPath (Default 4) {a:5} => {a:5})_",

    // EvalPath Drop
    R"_(EvalPath (Drop "a") 7 => 7)_",
    R"_(EvalPath (Drop "a") {foo:5} => {foo:5})_",
    R"_(EvalPath (Drop "a") {a:7} => {})_",
    R"_(EvalPath (Drop "a") {a:7, b:9} => {b:9})_",
    R"_(EvalPath (Drop "a") Nothing => Nothing)_",

    // EvalPath Keep
    R"_(EvalPath (Keep "a") 5 => 5)_",
    R"_(EvalPath (Keep "a") {foo:7} => {})_",
    R"_(EvalPath (Keep "a") {a:7, b:9} => {a:7})_",
    R"_(EvalPath (Keep "a") Nothing => Nothing)_",

    // EvalPath Obj
    R"_(EvalPath Obj 5 => Nothing)_",
    R"_(EvalPath Obj {a:7} => {a:7})_",

    // EvalPath Arr
    R"_(EvalPath Arr 5 => Nothing)_",
    R"_(EvalPath Arr [1,2,3] => [1,2,3])_",

    // EvalPath Field
    R"_(EvalPath (Field "a" Id) Nothing => Nothing)_",
    R"_(EvalPath (Field "a" Id) 5 => 5)_",
    R"_(EvalPath (Field "a" Id) {b:7} => {b:7})_",
    R"_(EvalPath (Field "a" Id) {a:5} => {a:5})_",
    R"_(EvalPath (Field "a" Const 7) 5 => {a:7})_",
    R"_(EvalPath (Field "a" Const 7) Nothing => {a:7})_",
    R"_(EvalPath (Field "a" Const Nothing) 5 => 5)_",
    R"_(EvalPath (Field "a" Const 7) {a:5} => {a:7})_",
    R"_(EvalPath (Field "a" Const 7) {b:7} => {b:7, a:7})_",

    // EvalPath Get
    R"_(EvalPath (Get "a" Id) 5 => Nothing)_",
    R"_(EvalPath (Get "a" Id) {b:7} => Nothing)_",
    R"_(EvalPath (Get "a" Id) {a:5} => 5)_",
    R"_(EvalPath (Get "a" Const 7) 5 => 7)_",
    R"_(EvalPath (Get "a" Const 7) {a:5} => 7)_",
    R"_(EvalPath (Get "a" Const 7) {b:7} => 7)_",

    // EvalPath At
    R"_(EvalPath (At 2 Id) 5 => Nothing)_",
    R"_(EvalPath (At 2 Id) [0,1] => Nothing)_",
    R"_(EvalPath (At 2 Id) [0,1,2] => 2)_",
    R"_(EvalPath (At 2 Default "foo") 5 => "foo")_",
    R"_(EvalPath (At 2 Default "foo") [0,1] => "foo")_",
    R"_(EvalPath (At 2 Default "foo") [0,1,2] => 2)_",

    // EvalPath Traverse
    R"_(EvalPath (Traverse Id) 5 => 5)_",
    R"_(EvalPath (Traverse Id) [1,2,3] => [1,2,3])_",
    R"_(EvalPath (Traverse Id) {a:7} => {a:7})_",
    R"_(EvalPath (Traverse Const 7) 5 => 7)_",
    R"_(EvalPath (Traverse Const 7) [1,2,3] => [7,7,7])_",
    R"_(EvalPath
      (Field "a" Traverse Field "b" Const 7)
      {a:[{b:1}, {b:2}, 3]} => {a:[{b:7}, {b:7}, {b:7}]})_",
    R"_(EvalPath
      (Field "a" Traverse Field "b" Id)
      {a:[{b:1}, {b:2}, 3]} => {a:[{b:1}, {b:2}, 3]})_",
    R"_(EvalPath
      (Field "a" Traverse Field "b" Id)
      {} => {})_",
    R"_(EvalPath
      (Field "a" Traverse Field "b" Id)
      {a: {b: 2}} => {a: {b: 2}})_",
    R"_(EvalPath
      (Field "a" Traverse Get "b" Id)
      {a:[{b:1}, {b:2}, 3]} => {a:[1, 2]})_",

    // EvalPath Composition
    R"_(EvalPath
      (Field "a" Const 7) * (Field "b" Const 9)
      Nothing => {a:7, b:9})_",

    R"_(EvalPath
      (Field "b" Const 9) * (Field "a" Const 7)
      Nothing => {b:9, a:7})_",

    R"_(EvalPath
      (Field "a" Const 7) * (Field "b" Const 9)
      {a:1, b:2, c:3} => {a:7, b:9, c:3})_",

    R"_(EvalPath
      (Field "a" Const 7) * (Field "b" Const 9) * (Keep "a","b")
      {a:1, b:2, c:3} => {a:7, b:9})_",
};

static const std::regex pattern(R"_(([\s\S]*)\s+=>\s+([\s\S]*))_");
TestCase parseTestCase(std::string testCase) {
    reflex::StdMatcher matcher(pattern, testCase);
    REQUIRE(matcher.matches());
    return {std::string(matcher[1].first, matcher[1].second),
            std::string(matcher[1].first, matcher[1].second)};
}

};  // namespace

TEST_CASE("Parse and Evaluation integration tests") {

    for (const auto& strTestCase : testCases) {
        INFO(strTestCase);

        auto testCase = parseTestCase(strTestCase);

        auto actualDocument = evaluate(testCase.expression);
        auto expectedDocument = evaluate(testCase.expectedDocument);

        REQUIRE(expectedDocument == actualDocument);
    }
}

}  // namespace mqlpath