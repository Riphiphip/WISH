%state IN_STRING
%{
    #include "scanner_defs.h"
%}
%option noyywrap
%option array
%option yylineno

%%
<INITIAL>{
    ["]                     { BEGIN(IN_STRING); }
    [\t\n\r\v ]+            ;
    .                       { return yytext[0]; }
}

<IN_STRING>{
    "\\\""                  { yymore(); }
    "\""                    { yyleng -= 1; yytext[yyleng] = 0; BEGIN(INITIAL); return STRING;}
    .                       { yymore(); }
}
%%
