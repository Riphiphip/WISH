#include "wish.h"
#include <sys/wait.h>
#include <stdio.h>
#include <fcntl.h>

int main(int argc, char **argv)
{
    bool is_script_executor = false;
    bool expect_cwd = false;
    bool should_exit = false;
    for (int i = 1; i < argc; i++)
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
        // Execute script
        pid_t result = executeScript(argv[i]);
        if (result == FORK_FAIL)
        {
            perror("Failed executing script");
        }
        else if (result != FORK_IN_CHILD)
        {
            exit(result);
        }
        else
        {
            is_script_executor = true;
        }
    }

    // Main loop
    while (true)
    {
        if (!is_script_executor)
        {
            char *cwd_string = getcwd(NULL, 512);
            printf("%s>:$ ", cwd_string);
            free(cwd_string);
        }
        size_t size = 256;
        char *input = (char *)malloc(sizeof(char *) * size);
        ssize_t input_length = getline(&input, &size, stdin);
        if (input_length < 0)
        {
            printf("Reading user input failed:\n\tError code: %ld\n", input_length);
            continue;
        }

        size_t arg_list_size = 8;
        size_t arg_count = 0;
        char **arg_list = initArgList(arg_list_size);

        char *input_target = NULL;
        char *output_target = NULL;

        bool abort = false;
        YY_BUFFER_STATE flex_buffer = yy_scan_string(input);
        int token = yylex();
        while (token != END)
        {
            // TASK A: print every token as captured by the lexer
            // There's more in the I/O redirects because they perform an extra lex
            #define TASK_A
            #ifdef TASK_A
            printf("%s\n", yytext);
            #endif
            switch (token)
            {
            case STRING:
            {
                if (arg_count >= arg_list_size)
                {
                    arg_list_size = arg_list_size * 2;
                    arg_list = resizeArgList(arg_list, arg_list_size);
                }
                argListInsert(yytext, arg_list, arg_count);
                arg_count++;
                break;
            }

            case REDIR_INPUT:
            {
                int redir_status = initRedirection();
                #ifdef TASK_A
                printf("%s\n", yytext);
                #endif
                if (redir_status == REDIR_TARGET_ERROR)
                {
                    abort = true;
                }
                // For input redirections it is an error for the input not to exist
                if (redir_status == REDIR_TARGET_NO_EXIST)
                {
                    perror("wish");
                    break;
                }

                if (input_target != NULL)
                    free(output_target);
                input_target = strdup(yytext);
                break;
            }

            case REDIR_OUTPUT:
            {
                int redir_status = initRedirection();
                #ifdef TASK_A
                printf("%s\n", yytext);
                #endif
                if (redir_status == REDIR_TARGET_ERROR)
                {
                    abort = true;
                }
                // For output redirections we create the file if it doesn't exist
                if (redir_status == REDIR_TARGET_NO_EXIST)
                {
                    // yytext contains the redirect target
                    int descriptor = open(yytext, O_CREAT);
                    close(descriptor);
                }

                // But we need write permission for it and it should be truncated when opened
                int descriptor = open(yytext, O_WRONLY|O_TRUNC);
                if (descriptor == -1 && errno == EACCES)
                {
                    perror("wish");
                    abort = true;
                    break;
                }
                close(descriptor);

                if (output_target != NULL)
                    free(output_target);
                output_target = strdup(yytext);
                break;
            }

            case END_OF_FILE:
            {
                should_exit = true;
                token = END;
                continue;
            }
            }
            token = yylex();
        }
        yy_delete_buffer(flex_buffer);
        free(input);

        // Error during lexing. The command should be aborted
        if (abort)
        {
            freeArgList(arg_list);
            continue;
        }

        //Skip command execution if nothing was written
        if (arg_count == 0) {
        }
        // Internal commands
        else if (!strcmp(CD_COMMAND, arg_list[0]))
        {
            if (chdir(arg_list[1])==-1){
                perror("wish");
            }
        }
        else if (!strcmp(EXIT_COMMAND, arg_list[0]))
        {
            exit(0);
        }
        else
        {
            // Execute the command with the given options in a child process
            pid_t child_pid = fork();
            if (child_pid == FORK_FAIL)
            {
                perror("wish");
                continue;
            }

            if (child_pid == FORK_IN_CHILD)
            {
                // I/O redirection
                int in_descriptor;
                int out_descriptor;
                if (input_target != NULL)
                {
                    in_descriptor = open(input_target, O_RDONLY);
                    dup2(in_descriptor, STDIN_FILENO);
                }

                if (output_target != NULL)
                {
                    out_descriptor = open(output_target, O_RDWR);
                    dup2(out_descriptor, STDOUT_FILENO);
                }

                int status = execvp(arg_list[0], arg_list);
                if (status == -1)
                {
                    perror(arg_list[0]);
                }

                if (input_target != NULL)
                    close(in_descriptor);
                if (output_target != NULL)
                    close(out_descriptor);

                exit(status);
            }
            else
            {
                // Wait for the command to finish execution
                waitpid(-1, NULL, 0);

                if (input_target != NULL)
                    free(input_target);
                if (output_target != NULL)
                    free(output_target);
            }
        }
        freeArgList(arg_list);
        if (should_exit)
        {
            exit(0);
        }
    }
    return 0;
}

/**
 * Initializes list for holding execution arguments for commands.
 * Returned pointer points to malloced area
 * and must be freed after use.
 * @param size Number of arguments that can be held by list.
 * @returns Pointer to argument list
 */
char **initArgList(size_t size)
{
    char **arg_list = (char **)calloc(size + 1, sizeof(char *));
    arg_list[0] = NULL;
    return arg_list;
}

/**
 * Resizes argument list
 * @param arg_list Argument list to be resized
 * @param new_size New list size
 * @returns Pointer to resized list
 */
char **resizeArgList(char **arg_list, size_t new_size)
{
    arg_list = realloc(arg_list, sizeof(char *) * (new_size + 1));
    return arg_list;
}

/**
 * Inserts string into arg_list at index.
 * Arguments should always be inserted at the end of the argument list.
 * Inserting anywhere else will cause a memory leak.
 * @param arg String to be inserted. Modifying passed string after execution will not affect the argument.
 * @param arg_list Argument list to insert string into
 * @param index Insertion index
 */
void argListInsert(char *arg, char **arg_list, long index)
{
    arg_list[index] = strdup(arg);
    arg_list[index + 1] = NULL;
}

/**
 * Free argument list and all associated arguments
 * @param arg_list Argument list to be freed
 */
void freeArgList(char **arg_list)
{
    int i = 0;
    while (arg_list[i] != NULL)
    {
        free(arg_list[i]);
        i++;
    }
    free(arg_list);
}

pid_t executeScript(char *file)
{
    pid_t pid = fork();
    if (pid == FORK_IN_CHILD)
    {
        fclose(stdin);
        stdin = fopen(file, "r");
    }
    else if (pid == FORK_FAIL)
    {
        perror("Script execution failed");
    }
    else
    {
        waitpid(-1, NULL, 0);
    }
    return pid;
}

/**
 * Lexes the redirection target and checks if the target exists.
 * @return 0 if the redirection target does not exist
 * @return 1 if the redirection target exists
 * @return -1 if the redirection target was not specified
 */
int initRedirection()
{
    int input_target = yylex();
    // It's a syntax error to not include a redirection target immediately after a I/O redirection
    if (input_target == END)
    {
        fprintf(stderr, "wish: I/O redirections must be followed by a redirection target\n");
        return REDIR_TARGET_ERROR;
    }

    int descriptor = open(yytext, O_RDONLY);
    if (descriptor == -1)
    {
        return REDIR_TARGET_NO_EXIST;
    }
    close(descriptor);

    return REDIR_TARGET_SUCCESS;
}