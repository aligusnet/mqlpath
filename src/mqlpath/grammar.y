%require  "3.8"
%language "C++"
%skeleton "lalr1.cc"

%defines "parser.h"
%output "parser.cpp"

%locations
%define api.location.file "location.h"

%define api.parser.class {Parser}
%define api.namespace {mqlpath}
%define api.value.type variant
%define api.token.constructor

%define parse.error verbose

%code requires{
#include <mqlpath/value.h>
#include <mqlpath/driver.h>

namespace mqlpath { class Lexer; }
}

%parse-param {Lexer& lexer}
%param {Driver* driver}

%code{
  #include "lexer.h"
  #undef yylex
  #define yylex lexer.lex
}

%define api.token.prefix {TOK_}
%token EOF 0 "end of file"
%token <std::string> INTEGER "integer"
%token <std::string> FLOAT "float"
%token <std::string> STRING "string"
%token <bool> BOOLEAN "boolean"
%token <std::string> IDENTIFIER "identifier"
%token NOTHING EVALPATH ID CONST DEFAULT LAMBDA DROP KEEP OBJ ARR FIELD GET AT TRAVERSE

%nterm <Expression> exp
%nterm <Value> value
%nterm <std::vector<Value>> valueList
%nterm <std::string> fieldName
%nterm <Object::Field> field
%nterm <std::vector<Object::Field>> fieldList
%nterm <Path> path
%nterm <std::vector<std::string>> stringList

%start start

%%

start: exp EOF       { driver->setAST(std::move($1)); }
  | start error EOF  { yyerrok; }
;

exp: value           { $$ = Expression::make<ConstantValue>(std::move($1), std::move(@$)); }
| EVALPATH path exp  { $$ = Expression::make<EvalPath>(std::move($2), std::move($3), std::move(@$)); }
;

value: NOTHING       { $$ = Value::make<NothingValue>(); }
 | INTEGER           { $$ = Value::make<ScalarValue>(Scalar(atoi($1.c_str()))); }
 | FLOAT             { $$ = Value::make<ScalarValue>(Scalar(atof($1.c_str()))); }
 | STRING            { $$ = Value::make<ScalarValue>(Scalar(std::move($1))); }
 | BOOLEAN           { $$ = Value::make<ScalarValue>(Scalar($1)); }
 | '[' ']'           { $$ = Value::make<ArrayValue>(std::vector<Value>{}); }
 | '[' valueList ']' { $$ = Value::make<ArrayValue>(std::move($2)); }
 | '{' '}'           { $$ = Value::make<ObjectValue>(Object{}); }
 | '{' fieldList '}' { $$ = Value::make<ObjectValue>(Object{std::move($2)}); }
;

valueList: value       { $$ = std::vector<Value>{std::move($1)}; }
 | valueList ',' value { $1.emplace_back(std::move($3));
                         $$ = std::move($1); }
;

field: fieldName ':' value { $$ = Object::Field{$1, $3}; }
;

fieldName : IDENTIFIER | STRING | INTEGER
;

fieldList: field       { $$ = std::vector<Object::Field>{std::move($1)}; }
 | fieldList ',' field { $1.emplace_back(std::move($3));
                         $$ = std::move($1); }
;

path: '(' path ')'       { $$ = std::move($2); }
 | ID                    { $$ = Path::make<IdPath>(); }
 | CONST exp             { $$ = Path::make<ConstPath>(std::move($2)); }
 | DEFAULT exp           { $$ = Path::make<DefaultPath>(std::move($2)); }
 | LAMBDA exp            { $$ = Path::make<LambdaPath>(std::move($2)); }
 | DROP stringList       { $$ = Path::make<DropPath>(std::move($2)); }
 | KEEP stringList       { $$ = Path::make<KeepPath>(std::move($2)); }
 | OBJ                   { $$ = Path::make<ObjPath>(); }
 | ARR                   { $$ = Path::make<ArrPath>(); }
 | FIELD fieldName path  { $$ = Path::make<FieldPath>(std::move($2), std::move($3)); }
 | GET fieldName path    { $$ = Path::make<GetPath>(std::move($2), std::move($3)); }
 | AT INTEGER path       { $$ = Path::make<AtPath>(atoi($2.c_str()), std::move($3)); }
 | TRAVERSE path         { $$ = Path::make<TraversePath>(std::move($2)); }
;

stringList: STRING       { $$ = std::vector<std::string>{std::move($1)}; }
 | stringList ',' STRING { $1.emplace_back(std::move($3));
                           $$ = std::move($1); }
;
%%

void mqlpath::Parser::error(const location& loc, const std::string& msg)
{
  driver->getErrors().append("parser", msg, loc);
  if (lexer.size() == 0)      // if token is unknown (no match)
    lexer.matcher().winput(); // skip character
}