%{
#include <types.h>
#include <y.tab.h>
void yyerror(const char *e) { std::cerr << e; }
extern "C" int yywrap() { return 1; }
%}

ID [a-z]+
AND "&&"
OR "||"
NOT "!"

%%
[[:space:]]+
{ID} yylval.s = yytext; return ID;
{AND} return AND;
{OR} return OR;
{NOT} return NOT;

%%
