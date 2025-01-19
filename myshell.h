#ifndef MYSHELL_H_
#define MYSHELL_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_LENGTH 100

typedef struct {
    char** tokens;
    int count;
    int size;
} TokenList;

// here are all the functions which we are using in our program to make a basic interactive shell.
int execute_command(TokenList* tokens);
void execute_single_command(char* args[]);
int process_and_execute_command(TokenList* tokens);
int read_and_execute_commands(int input_fd, int output_fd);
void execute_builtin_command(char* args[], int* should_exit);

void setup_redirection(int* input_fd, int* output_fd);
int matches_pattern(char* filename, char* prefix, char* suffix);
void handle_wildcards(TokenList* tokenList);
void remove_first_token(TokenList* tokenList);

void interactive_mode();
void batch_mode(char* script_path);

// 4 built-in commands
int change_directory(char* path);
int print_working_directory();
int execute_which(char* program_name);
// exit is handled in myshell.c

void tokenize_input(char* input, int bytes, TokenList* tokenList);
void init_token_list(TokenList* tokenList);
void free_token_list(TokenList* tokenList);
void print_tokens(TokenList* tokenList);

#endif
