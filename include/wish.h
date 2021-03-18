#pragma once

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "scanner.h"

/**
 * Initializes list for holding execution arguments for commands.
 * Returned pointer points to malloced area
 * and must be freed after use.
 * @param size Number of arguments that can be held by list.
 * @returns Pointer to argument list
 */
char **initArgList(size_t size);

/**
 * Resizes argument list
 * @param arg_list Argument list to be resized
 * @param new_size New list size
 * @returns Pointer to resized list
 */
char **resizeArgList(char **arg_list, size_t new_size);

/**
 * Inserts string into arg_list at index.
 * Arguments should always be inserted at the end of the argument list.
 * Inserting anywhere else will cause a memory leak.
 * @param arg String to be inserted. Modifying passed string after execution will not affect the argument.
 * @param arg_list Argument list to insert string into
 * @param index Insertion index
 */
void argListInsert(char *arg, char **arg_list, long index);

/**
 * Free argument list and all associated arguments
 * @param arg_list Argument list to be freed
 */
void freeArgList(char **arg_list);

pid_t executeScript(char* file);

#define REDIR_TARGET_NO_EXIST 0
#define REDIR_TARGET_SUCCESS 1
#define REDIR_TARGET_ERROR -1

/**
 * Lexes the redirection target and checks if the target exists.
 * @return 0 if the redirection target does not exist
 * @return 1 if the redirection target exists
 * @return -1 if the redirection target was not specified
 */
int initRedirection();

#define BASE_ARGLIST_SIZE 8
#define CD_COMMAND "cd"
#define EXIT_COMMAND "exit"
#define EXECUTE_SCRIPT_COMMAND "source"
