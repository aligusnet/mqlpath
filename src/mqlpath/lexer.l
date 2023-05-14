%top{
#include <mqlpath/driver.h>
#include "parser.h" 
#include "location.h"
}

%option c++ noyywrap nodefault
%option outfile="lexer.cpp" header-file="lexer.h"
%option bison-complete
%option bison-cc-namespace=mqlpath
%option bison-cc-parser=Parser
%option bison-locations

%option exception="mqlpath::Parser::syntax_error(location(), \"Unknown token.\")"

%option namespace=mqlpath
%option lexer=Lexer
%option params="Driver* driver"

float -?([0-9]*[.])?[0-9]+
string \"(\\.|\"\"|[^"\n])*\"

%%
"//".*  
[ \t\n]  /* ignore whitespace */ 
 /* single character ops */
"{" |
"}" |
"[" |
"]" |
":" |
"(" |
")" |
"*" |
","   { return mqlpath::Parser::symbol_type(chr(), location()); }

"Nothing"  { return mqlpath::Parser::make_NOTHING(location()); }
"EvalPath" { return mqlpath::Parser::make_EVALPATH(location()); }
"Id"       { return mqlpath::Parser::make_ID(location()); }
"Const"    { return mqlpath::Parser::make_CONST(location()); }
"Default"  { return mqlpath::Parser::make_DEFAULT(location()); }
"Lambda"   { return mqlpath::Parser::make_LAMBDA(location()); }
"Drop"     { return mqlpath::Parser::make_DROP(location()); }
"Keep"     { return mqlpath::Parser::make_KEEP(location()); }
"Obj"      { return mqlpath::Parser::make_OBJ(location()); }
"Arr"      { return mqlpath::Parser::make_ARR(location()); }
"Field"    { return mqlpath::Parser::make_FIELD(location()); }
"Get"      { return mqlpath::Parser::make_GET(location()); }
"At"       { return mqlpath::Parser::make_AT(location()); }
"Traverse" { return mqlpath::Parser::make_TRAVERSE(location()); }

/* scalar values */
[0-9]+ { return mqlpath::Parser::make_INTEGER(str(), location()); }
{float} { return mqlpath::Parser::make_FLOAT(str(), location()); }
{string} { return mqlpath::Parser::make_STRING(str().substr(1, size()-2), location()); }
\"(\\.|[^"\n])*$    { mqlpath::Parser::syntax_error(location(), std::string("Unterminated string ") + text()); }
"true"  { return mqlpath::Parser::make_BOOLEAN(true, location()); }
"false" { return mqlpath::Parser::make_BOOLEAN(false, location()); }

/* names */
[a-zA-Z][a-zA-Z0-9]*  { return mqlpath::Parser::make_IDENTIFIER(text(), location()); }

<<EOF>> {return mqlpath::Parser::make_EOF(location()); }
%%
