#include "wish.h"

int main(int argc, char **argv)
{
    bool expect_cwd = false;
    for (int i = 0; i < argc; i++)
    {
        if (!strcmp(argv[i], "-p"))
        {
            expect_cwd = true;
            continue;
        }
        if (expect_cwd)
        {
            int result = chdir(argv[i]);
            if (result)
            {
                printf("Invalid path provided:\n\t%s\n", argv[i]);
                exit(1);
            }
            expect_cwd = false;
            continue;
        }
    }

    while (true)
    {
        char *cwd_string = getcwd(NULL, 512);
        printf("%s>:$ ", cwd_string);
        free(cwd_string);
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