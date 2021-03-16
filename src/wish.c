#include "wish.h"
#include <sys/wait.h>
#include <stdio.h>
#include <fcntl.h>

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
        printf("%s (%d)>:$ ", cwd_string, getpid());
        free(cwd_string);
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

        YY_BUFFER_STATE flex_buffer = yy_scan_string(input);
        int token = yylex();
        while (token != END)
        {
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
                    int redir_stat = initRedirection();
                    // For input redirections, it is an error for the target not to exist
                    if (redir_stat == REDIR_TARGET_NO_EXIST)
                    {
                        perror(yytext);
                    }
                    // TODO: Redirect the file to stdin using dup(2)/dup2(2) IN THE CHILD EXECUTING THE COMMAND
                    break;
                }

                case REDIR_OUTPUT:
                {
                    int redir_stat = initRedirection();
                    // TODO: Redirecting stdout to the output target
                    // * Create the output file if it doesn't exist
                    // * Actually redirect stdout using dup(2)/dup2(2) IN THE CHILD EXECUTING THE COMMAND
                    break;
                }
            }
            token = yylex();
        }
        yy_delete_buffer(flex_buffer);
        free(input);

        // Execute the command with the given options in a child process
        pid_t child_pid = fork();
        if (child_pid == FORK_FAIL)
        {
            exit(EXIT_FAILURE);
        }

        if (child_pid == FORK_IN_CHILD)
        {
            int status = execvp(arg_list[0], arg_list);
            if (status == -1)
            {
                perror(arg_list[0]);
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

        freeArgList(arg_list);
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

/**
 * Lexes the redirection target and checks if the target exists.
 * @return 0 if the redirection target does not exist
 * @return 1 otherwise
 */
int initRedirection()
{
    int input_target = yylex();
    // It's a syntax error to not include a redirection target immediately after a I/O redirection
    if (input_target == END)
    {
        fprtintf(stderr, "wish: I/O redirections must be followed by a redirection target");
        exit(1);
    }

    int descriptor = open(yytext, O_RDONLY);
    if (descriptor == -1)
    {
        return REDIR_TARGET_NO_EXIST;
    }
    close(descriptor);

    return REDIR_TARGET_SUCCESS;
}