#include "wish.h"
#include <sys/wait.h>
#include <errno.h>
#include <stdio.h>

#define FORK_FAIL -1
#define FORK_IN_CHILD 0

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
        char *input = (char *)malloc(sizeof(char *) * size);
        ssize_t input_length = getline(&input, &size, stdin);
        if (input_length < 0)
        {
            printf("Reading user input failed:\n\tError code: %ld\n", input_length);
            continue;
        }

        YY_BUFFER_STATE flex_buffer = yy_scan_string(input);
        int token = yylex();
        char *command = NULL;

        int option_count = 1;
        char *options[256] = {NULL};
        while (token != END)
        {
            switch (token)
            {
                case STRING:
                {
                    // First token is always the command
                    if (command == NULL)
                    {
                        command = (char *)malloc(sizeof(char *) * yyleng);
                        strcpy(command, yytext);
                        // Command options cannot be NULL, and by convention, argv[0] is the name of the command/program being run
                        options[0] = command;

                        break;
                    }

                    // The rest are options
                    char *option = (char *)malloc(sizeof(char *) * yyleng);
                    strcpy(option, yytext);
                    options[option_count++] = option;

                    break;
                }
            }
            token = yylex();
        }
        yy_delete_buffer(flex_buffer);

        // Execute the command with the given options in a child process
        pid_t child_pid = fork();
        if (child_pid == FORK_FAIL)
        {
            exit(EXIT_FAILURE);
        }

        if (child_pid == FORK_IN_CHILD)
        {
            int status = execvp(command, options);
            if (status == -1)
            {
                perror(command);
            }
            exit(status);
        }
        else
        {
            // Wait for the command to finish execution
            #ifdef DEBUG
            int status;
            waitpid(-1, &status, 0);
            printf("exit %d\n", status);
            #else
            waitpid(-1, NULL, 0);
            #endif
        }

        // Don't leak memory :)
        // Note: the command is freed in the first iteration
        for (int i = 0; i < option_count; i++)
        {
            free(options[i]);
        }
        free(input);
    }
    return 0;
}