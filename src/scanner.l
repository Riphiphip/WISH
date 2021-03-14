%state IN_STRING
%{
    enum token_types {
        STRING,
        END
    };
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
    "\""                    { BEGIN(INITIAL); return STRING;}
    .                       { yymore(); }
}
%%

// Debug code
int main(void) {
    return 0;
}