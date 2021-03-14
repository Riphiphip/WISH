#include "wish.h"

int main(void) {
    int token = yylex();
    while(token != END){
        switch(token) {
            case STRING:{
                printf("%s\n",yytext);
                break;
            }
        }
        token = yylex();
    }
    return 0;
}