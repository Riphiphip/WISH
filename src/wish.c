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

    // Main loop
    while (true)
    {
        char *cwd_string = getcwd(NULL, 512);
        printf("%s>:$ ", cwd_string);
        free(cwd_string);
        size_t size = 256;
        char *input = (char *)malloc(sizeof(char) * size);
        ssize_t input_length = getline(&input, &size, stdin);
        if (input_length < 0)
        {
            printf("Reading user input failed:\n\tError code: %ld\n", input_length);
            continue;
        }
        YY_BUFFER_STATE flex_buffer = yy_scan_string(input);
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
        yy_delete_buffer(flex_buffer);
    }
    return 0;
}