#include "wish.h"

int main(void)
{
    while (1)
    {
        printf(">:$ ");
        int token = yylex();
        while (token != END)
        {
            switch (token)
            {
            case STRING:
            {
                printf("%s\n", yytext);
                break;
            }
            }
            token = yylex();
        }
    }
    return 0;
}