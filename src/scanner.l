%state IN_STRING
%{
    #include "scanner_defs.h"
%}
%option noyywrap
%option array
%option yylineno

%%
<INITIAL>{
    <<EOF>>                 { return END; }
    \n                      { return END; }
    ["]                     { BEGIN(IN_STRING); }
    [<]                     { return REDIR_INPUT; }
    [>]                     { return REDIR_OUTPUT; }
    [\t\r\v ]+              ;
    [^"\n\t\r\v ]+          { return STRING; }
}

<IN_STRING>{
    "\\\""                  { yymore(); }
    "\""                    { yyleng -= 1; yytext[yyleng] = 0; BEGIN(INITIAL); return STRING;}
    .                       { yymore(); }
}
%%
