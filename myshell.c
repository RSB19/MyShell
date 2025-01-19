#include "myshell.h"

#ifndef DEBUG
#define DEBUG 0
#endif

int prev_command_status;

// Tokenize input string and store tokens in a TokenList
void tokenize_input(char* input, int bytes, TokenList* tokenList) {
    init_token_list(tokenList); // Make sure TokenList is initialized
    int inputStart = 0;

    for (ssize_t m = 0; m < bytes; m++) {
        if (isspace(input[m]) || input[m] == '\0') {
            if (m > inputStart) {
                // Word found, add it as a token
                tokenList->tokens[tokenList->count++] = strndup(&input[inputStart], m - inputStart);
                if (tokenList->count >= tokenList->size) {
                    // Need more space for tokens
                    tokenList->size *= 2;
                    char** temp = realloc(tokenList->tokens, tokenList->size * sizeof(char*));
                    if (!temp) {
                        perror("realloc");
                        exit(EXIT_FAILURE);
                    }
                    tokenList->tokens = temp;
                }
            }
            inputStart = m + 1;
        } else if (input[m] == '>' || input[m] == '<' || input[m] == '|') {
            if (m > inputStart) {
                // Add the word before the special token as a token
                tokenList->tokens[tokenList->count++] = strndup(&input[inputStart], m - inputStart);
                if (tokenList->count >= tokenList->size) {
                    // Need more space for tokens
                    tokenList->size *= 2;
                    char** temp = realloc(tokenList->tokens, tokenList->size * sizeof(char*));
                    if (!temp) {
                        perror("realloc");
                        exit(EXIT_FAILURE);
                    }
                    tokenList->tokens = temp;
                }
            }
            // Add the special token (`>`, `<`, `|`) as a separate token
            tokenList->tokens[tokenList->count++] = strndup(&input[m], 1);
            if (tokenList->count >= tokenList->size) {
                // Need more space for tokens
                tokenList->size *= 2;
                char** temp = realloc(tokenList->tokens, tokenList->size * sizeof(char*));
                if (!temp) {
                    perror("realloc");
                    exit(EXIT_FAILURE);
                }
                tokenList->tokens = temp;
            }
            inputStart = m + 1; // Move past the special token
        }
    }
    // Handle the case where the last character is not a special token
    if (inputStart < bytes) {
        tokenList->tokens[tokenList->count++] = strndup(&input[inputStart], bytes - inputStart);
        if (tokenList->count >= tokenList->size) {
            // Need more space for tokens
            tokenList->size *= 2;
            char** temp = realloc(tokenList->tokens, tokenList->size * sizeof(char*));
            if (!temp) {
                perror("realloc");
                exit(EXIT_FAILURE);
            }
            tokenList->tokens = temp;
        }
    }
}

void init_token_list(TokenList* tokenList) {
    tokenList->count = 0;
    tokenList->size = 10; // Initial size, can grow as needed
    tokenList->tokens = malloc(tokenList->size * sizeof(char*));
    if (tokenList->tokens == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
}

// Utility function to check if a filename matches the wildcard pattern
int matches_pattern(char* filename, char* prefix,  char* suffix) {
    size_t filename_len = strlen(filename);
    size_t prefix_len = strlen(prefix);
    size_t suffix_len = strlen(suffix);

    // Filename must be at least as long as prefix + suffix
    if (filename_len < prefix_len + suffix_len) return 0;

    // Check if filename starts with prefix
    if (strncmp(filename, prefix, prefix_len) != 0) return 0;

    // Check if filename ends with suffix
    if (suffix_len > 0 && strncmp(filename + filename_len - suffix_len, suffix, suffix_len) != 0) return 0;

    return 1;
}

void handle_wildcards(TokenList* tokenList) {
    for (int i = 0; i < tokenList->count; ++i) {
        char* token = tokenList->tokens[i];
        if (strchr(token, '*') != NULL) { // Token contains a wildcard
            char* lastSlash = strrchr(token, '/');
            char dirPath[256] = ".";
            char* pattern = token;
            if (lastSlash != NULL) {
                snprintf(dirPath, lastSlash - token + 1, "%s", token);
                pattern = lastSlash + 1;
            }

            DIR* dir = opendir(dirPath);
            if (!dir) continue;

            struct dirent* entry;
            char** matchesArray = NULL;
            int matches = 0;
            while ((entry = readdir(dir)) != NULL) {
                if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

                char prefix[256] = {0}, suffix[256] = {0};
                char* asteriskPos = strchr(pattern, '*');
                if (asteriskPos != NULL) {
                    if (asteriskPos != pattern) strncpy(prefix, pattern, asteriskPos - pattern);
                    if (*(asteriskPos + 1) != '\0') strcpy(suffix, asteriskPos + 1);
                }

                if (matches_pattern(entry->d_name, prefix, suffix)) {
                    // Resize matchesArray and add the new match
                    matchesArray = realloc(matchesArray, (matches + 1) * sizeof(char*));
                    char fullPath[512];
                    if (strcmp(dirPath, ".") != 0) snprintf(fullPath, sizeof(fullPath), "%s/%s", dirPath, entry->d_name);
                    else strncpy(fullPath, entry->d_name, sizeof(fullPath));
                    matchesArray[matches] = strdup(fullPath);
                    matches++;
                }
            }
            closedir(dir);

            // Replace the original token with the matches found, if any
            if (matches > 0) {
                free(tokenList->tokens[i]);
                tokenList->tokens[i] = matchesArray[0]; // Replace with the first match

                // Append any additional matches
                for (int j = 1; j < matches; j++) {
                    // Resize tokenList to accommodate the new token
                    tokenList->tokens = realloc(tokenList->tokens, (tokenList->count + 1) * sizeof(char*));
                    // Shift tokens to make space for the new token
                    for (int k = tokenList->count; k > i + j; k--) {
                        tokenList->tokens[k] = tokenList->tokens[k - 1];
                    }
                    tokenList->tokens[i + j] = matchesArray[j]; // Insert the match
                    tokenList->count++;
                }

                // Adjust loop counter to account for added tokens
                i += matches - 1;
            }

            // Clean up
            if (matchesArray != NULL && matches == 1) {
                free(matchesArray); // Only allocated array, not strings
            } else if (matchesArray != NULL) {
                free(matchesArray); // All strings were moved to tokenList, just free the container
            }
        }
    }
}

void setup_redirection(int* input_fd, int* output_fd) {
    // Input redirection
    if (*input_fd != STDIN_FILENO) {
        if (dup2(*input_fd, STDIN_FILENO) == -1) {
            perror("dup2");
            exit(EXIT_FAILURE);
        }
        close(*input_fd);
    }

    // Output redirection
    if (*output_fd != STDOUT_FILENO) {
        if (dup2(*output_fd, STDOUT_FILENO) == -1) {
            perror("dup2");
            exit(EXIT_FAILURE);
        }
        close(*output_fd);
    }
}

int is_builtin_command(char* command) {
    return (strcmp(command, "cd") == 0 || strcmp(command, "pwd") == 0 || 
            strcmp(command, "which") == 0 || strcmp(command, "exit") == 0);
}

void execute_builtin_command(char* args[], int* should_exit) {
    
    if (strcmp(args[0], "exit") == 0) {
        *should_exit = 1;
        // Print all arguments received with exit command
        for (int i = 1; args[i] != NULL; i++) {
            printf("%s \n", args[i]);
        }
        printf("Exiting the shell. Goodbye!\n");
    } else if (strcmp(args[0], "cd") == 0) {
        if(change_directory(args[1])){
           prev_command_status = 1;
        }
        else{
            prev_command_status = 0;
        }
    } else if (strcmp(args[0], "pwd") == 0) {
        if(print_working_directory()){
        prev_command_status = 1;
        }
	    else{
            prev_command_status = 0;
            }
    } else if (strcmp(args[0], "which") == 0) {
        if(execute_which(args[1])){
            prev_command_status = 1;
        }
	   else{
            prev_command_status = 0;
            }
    } else {
        printf("Error: Unknown builtin command\n");
        prev_command_status = 0;
    }
}

void print_tokens(TokenList* tokenList) {
    printf("Tokens:\n");
    for (int i = 0; i < tokenList->count; i++) {
        printf("%s ", tokenList->tokens[i]);
    }
    printf("\n");
}

void free_token_list(TokenList* tokenList) {
     for (int i = 0; i < tokenList->count; i++) {
        free(tokenList->tokens[i]);
    }
    free(tokenList->tokens);
    tokenList->tokens = NULL;
    tokenList->count = 0;
    tokenList->size = 0;
}

void execute_single_command(char* args[]) {
    char* directories[] = {"/usr/local/bin", "/usr/bin", "/bin"};
    char filename[MAX_LENGTH];
    struct stat st;
    int executed = 0;

    if (args[0] == NULL) {
        printf("Error: No command provided\n");
	prev_command_status = 0;
        exit(EXIT_FAILURE);
    }

    // Check if the command includes a directory (i.e., it's a path to an executable)
    if (strchr(args[0], '/') != NULL) {
        if (stat(args[0], &st) == 0 && S_ISREG(st.st_mode) && (st.st_mode & S_IXUSR)) {
	        prev_command_status = 1;
            executed = 1;
            execv(args[0], args);
            // If execv returns, it failed
            perror("execv");
            exit(EXIT_FAILURE);
        } else {
            printf("Error: program executable not found: %s\n", args[0]);
            prev_command_status = 0;
            executed = 0;
            exit(EXIT_FAILURE);
        }
    } else {
        // Search for the command in the specified directories
        for (int i = 0; i < sizeof(directories) / sizeof(char*); i++) {
            snprintf(filename, MAX_LENGTH, "%s/%s", directories[i], args[0]);

            if (stat(filename, &st) == 0 && S_ISREG(st.st_mode) && (st.st_mode & S_IXUSR)) {
		    executed = 1;
		    prev_command_status = 1;
		    //printf("%d\n", prev_command_status);
            execv(filename, args);
		    perror("execv");
		    exit(EXIT_FAILURE);
            break; // Command executed, break out of the loop
            }
        }

        if (!executed) {
            prev_command_status = 0;
            printf("Error: command not found: %s\n", args[0]);
        }
    }
}

int change_directory(char* path) {
    if (path == NULL) {
        path = getenv("HOME");
        return 1;
    }
    if (chdir(path) == -1) {
        perror("cd");
        return 0;
    }
}

int print_working_directory() {
    char* cwd = getcwd(NULL, 0);
    if (cwd == NULL) {
        perror("pwd");
        return 0;
    } else {
        printf("%s\n", cwd);
        free(cwd);
        return 1;
    }
}

int execute_which(char* program_name) {
    if (program_name == NULL) {
        printf("Error: missing program name for which command\n");
        return 0;
    }

    char* directories[] = {"/usr/local/bin", "/usr/bin", "/bin"};
    struct stat st;
    for (int i = 0; i < sizeof(directories) / sizeof(char*); i++) {
        char filename[MAX_LENGTH];
        snprintf(filename, MAX_LENGTH, "%s/%s", directories[i], program_name);
	if(stat(filename, &st) == 0 && S_ISREG(st.st_mode) && (st.st_mode & S_IXUSR)) {
            printf("%s\n", filename);
            return 1;
        }
    }
    printf("Error: command not found: %s \n", program_name);
    return 0;
}

int execute_command(TokenList* tokens) {
    int should_exit = 0; // Flag to indicate if the shell should exit after command execution

    // Variables for handling input and output redirection
    int input_fd = STDIN_FILENO, output_fd = STDOUT_FILENO;
    int original_stdout = dup(STDOUT_FILENO);  // Duplicate the original stdout
    int original_stdin = dup(STDIN_FILENO);    // Duplicate the original stdin

    char** clean_args = malloc(tokens->size * sizeof(char*));
    if (!clean_args) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    int clean_arg_index = 0;

    // Process tokens to prepare command execution, filtering out redirection and pipe tokens
    for (int i = 0; i < tokens->count; i++) {
        if (strcmp(tokens->tokens[i], "<") == 0) {
            // Handle input redirection
            input_fd = open(tokens->tokens[i + 1], O_RDONLY);
            if (input_fd == -1){
                perror("open");
                free(clean_args);
                exit(EXIT_FAILURE);
            } 
            i++; // Skip the filename token
        } else if (strcmp(tokens->tokens[i], ">") == 0) {
            // Handle output redirection
            output_fd = open(tokens->tokens[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0640);
            if (output_fd == -1){
                perror("open");
                free(clean_args);
                exit(EXIT_FAILURE);
            } 
            i++; // Skip the filename token
        } else {
            // Add non-redirection tokens to clean_args
            clean_args[clean_arg_index++] = tokens->tokens[i];
        }
    }
    clean_args[clean_arg_index] = NULL; // Ensure the arguments are null-terminated

    // Identifying if there's a pipe in the command
    int pipe_index = -1;
    for (int i = 0; i < clean_arg_index; i++) {
        if (strcmp(clean_args[i], "|") == 0) {
            pipe_index = i;
            clean_args[i] = NULL; // Null-terminate the first part of the command
            break;
        }
    }

    if (pipe_index != -1) {
        // Pipes are present in the command
        int status1, status2;

        // Pipe creation
        int pipe_fd[2];
        if (pipe(pipe_fd) < 0) perror("pipe");

        if (fork() == 0) {
            // First child: handles the command before the pipe
            dup2(pipe_fd[1], STDOUT_FILENO); // Connect stdout to the write end of the pipe
            close(pipe_fd[0]); // Close the read end of the pipe
            
            // Apply input redirection for the first command if specified
            if (input_fd != STDIN_FILENO) {
                dup2(input_fd, STDIN_FILENO);
                close(input_fd);
            }

            // Execute the first part of the command
            if (is_builtin_command(clean_args[0])) {
                execute_builtin_command(clean_args, &should_exit);
                exit(prev_command_status);
            } else {
                execute_single_command(clean_args);
                exit(prev_command_status);
            }
        }

        // Parent process: close write end of the pipe
        close(pipe_fd[1]);
        wait(&status1); 

        // Execute the second part of the command only if there's a second command
        if (clean_args[pipe_index + 1] != NULL) {
            if (fork() == 0) {
                // Second child: handles the command after the pipe
                // Apply output redirection for the second command if specified, overriding the pipe
                if (output_fd != STDOUT_FILENO) {
                    dup2(output_fd, STDOUT_FILENO);
                    close(output_fd);
                }

                // Close the write end of the pipe in the second child process, as it's not needed
                close(pipe_fd[1]);

                // Here's where you need to apply the logic to prioritize input redirection over the pipe
                if (input_fd != STDIN_FILENO) {
                    // Input redirection specified, so it takes precedence over the pipe
                    dup2(input_fd, STDIN_FILENO);
                    close(input_fd);
                } else {
                    // No input redirection specified, so use the pipe as the input source
                    dup2(pipe_fd[0], STDIN_FILENO);
                }
                // It's important to close the pipe_fd[0] after duplicating if it's not being used for input
                close(pipe_fd[0]);

                // Execute the second part of the command
                char** second_part_args = &clean_args[pipe_index + 1];
                if (is_builtin_command(second_part_args[0])) {
                    execute_builtin_command(second_part_args, &should_exit);
                    exit(prev_command_status);
                } else {
                    execute_single_command(second_part_args);
                    exit(prev_command_status);
                }
            }
        }

        // Parent process: close read end of the pipe and wait for children
        close(pipe_fd[0]);
        if (clean_args[pipe_index + 1] != NULL) {
            wait(&status2); // Wait for the second child
        }

        // Check if any of the children exited with an error
        if (WIFEXITED(status1) && WEXITSTATUS(status1) != EXIT_SUCCESS) {
            should_exit = 1;
        }
        if (clean_args[pipe_index + 1] != NULL && WIFEXITED(status2) && WEXITSTATUS(status2) != EXIT_SUCCESS) {
            should_exit = 1;
        }
    }
   else {
        // Handling for commands without a pipe
        if (is_builtin_command(clean_args[0])) {
            // Apply redirection for built-in commands
            setup_redirection(&input_fd, &output_fd);  // Ensure redirection is set up before execution
            // Execute the built-in command
            execute_builtin_command(clean_args, &should_exit);

            dup2(original_stdin, STDIN_FILENO);
            close(original_stdin);
            dup2(original_stdout, STDOUT_FILENO);
            close(original_stdout);
        } else {
            // Execution of commands without a pipe
            pid_t pid = fork();
            if (pid == 0) {
                // Child process: Apply redirection
                if (input_fd != STDIN_FILENO) {
                    dup2(input_fd, STDIN_FILENO);
                    close(input_fd);
                }
                if (output_fd != STDOUT_FILENO) {
                    dup2(output_fd, STDOUT_FILENO);
                    close(output_fd);
                }

                // Execute the command
                if (is_builtin_command(clean_args[0])) {
                    execute_builtin_command(clean_args, &should_exit);
                    exit(prev_command_status);  // Exiting after built-in command execution
                } else {
                    execute_single_command(clean_args);  // For external commands
                    if(prev_command_status == 0){
                        exit(EXIT_FAILURE);
                    }
                    else{
                    exit(prev_command_status);
                    }  
                    // Ensure to exit if execv was successful
                }
            } else if (pid > 0) {
                // Parent process waits for the child process to complete
                int status;
                wait(&status);
                // Process exit status
                if (WIFEXITED(status)) {
                    int exitStatus = WEXITSTATUS(status);    
                    if (exitStatus == 0) {
                        prev_command_status = 1; // Success
                    } else {
                        prev_command_status = 0; // Failure
                    }
                }
            } else {
                prev_command_status = 0;
                perror("fork");  // Forking failed
            }
         }
    }

    free(clean_args);

    if (should_exit) {
        // Exit the shell if the exit flag is set
        exit(EXIT_SUCCESS);
    }

    return prev_command_status;
}


void remove_first_token(TokenList* tokenList) {
    if (tokenList->count <= 0) return;
    free(tokenList->tokens[0]); // Free the memory of the first token
    for (int i = 1; i < tokenList->count; i++) {
        tokenList->tokens[i - 1] = tokenList->tokens[i];
    }
    tokenList->count--;
}

int process_and_execute_command(TokenList* tokenList) {
    int executeCommand = 1; // Default to executing the command

    handle_wildcards(tokenList);

    // Conditional logic for handling 'then' and 'else'
    if (strcmp(tokenList->tokens[0], "then") == 0) {
        if (prev_command_status == 1) {
            executeCommand = 1;
        } else if (prev_command_status == 0){
            executeCommand = 0;
        }
        remove_first_token(tokenList);
    } else if (strcmp(tokenList->tokens[0], "else") == 0) {
        if (prev_command_status == 0) {
            executeCommand = 1;
        } else if(prev_command_status == 1){
            executeCommand = 0;
        }
        remove_first_token(tokenList);
    }

    if (executeCommand) {
        // Check for special commands like 'exit', 'cd', 'which', 'pwd'
        if (strcmp(tokenList->tokens[0], "exit") == 0 || strcmp(tokenList->tokens[0], "cd") == 0 || strcmp(tokenList->tokens[0], "which") == 0 || strcmp(tokenList->tokens[0], "pwd") == 0) {
            if(DEBUG) {
                print_tokens(tokenList);
            } 
            prev_command_status = execute_command(tokenList);
            if (strcmp(tokenList->tokens[0], "exit") == 0) {
                // Special handling for 'exit' command, including redirection
                // Assuming execute_command handles the redirection properly for 'exit'
                // If there's additional cleanup or messages, handle it before this point
                free_token_list(tokenList);
                exit(prev_command_status); // Use the status from execute_command if you wish to return a specific exit code
            }
            free_token_list(tokenList);
            return 1;
        } else {
            if(DEBUG){
                print_tokens(tokenList);
            } 
            prev_command_status = execute_command(tokenList);
            free_token_list(tokenList);
            return 1;
        }
    } else {
        free_token_list(tokenList);
        return 1; // Ignore the command since prev_command_status was 0 and continue
    }
}

int read_and_execute_commands(int input_fd, int output_fd) {
    int capacity = MAX_LENGTH; // Start with MAX_LENGTH but prepare to increase as needed
    char* line = malloc(capacity * sizeof(char));
    if (!line) {
        perror("malloc");
        return EXIT_FAILURE; // Allocation failed
    }
    int total_bytes_read = 0;
    int interact_r = 0;
    int batch_r = 0;
    TokenList tokenList;

    while (1) {
        if (total_bytes_read >= capacity - 1) {
            // Increase buffer size
            capacity *= 2;
            char* new_line = realloc(line, capacity * sizeof(char));
            if (!new_line) {
                free(line); // Free the original line before returning
                return -1; // Re-allocation failed
            }
            line = new_line;
        }

        int bytes = read(input_fd, line + total_bytes_read, capacity - total_bytes_read - 1);
        if (bytes <= 0) break; // End of file or error

        total_bytes_read += bytes;
        line[total_bytes_read] = '\0'; // Ensure the buffer is null-terminated

        if (input_fd == STDIN_FILENO && isatty(STDIN_FILENO)) {
            // Interactive mode
            tokenize_input(line, total_bytes_read, &tokenList);
            interact_r = process_and_execute_command(&tokenList);
            free(line);
            return interact_r;
        } else {
            // Batch mode, now with dynamically adjusted buffer sizes
            ssize_t start = 0;
            for (ssize_t r = 0; r < total_bytes_read; r++) {
                if (line[r] == '\n') {
                    tokenize_input(&line[start], r - start, &tokenList);
                    batch_r = process_and_execute_command(&tokenList);
                    start = r + 1;
                    free_token_list(&tokenList);
                    if (batch_r == -1) {
                        if (input_fd != STDIN_FILENO) {
                            close(input_fd);
                        }
                        free(line);
                        return batch_r;
                    }
                }
            }
            // If there were any commands processed, adjust the buffer for any remaining input
            if (start > 0) {
                memmove(line, line + start, total_bytes_read - start);
                total_bytes_read -= start;
            }
        }
    }
    // Close input_fd if it's not standard input and free the line buffer
    if (input_fd != STDIN_FILENO) {
        close(input_fd);
    }
    free(line);
    return -1; // Indicates end of input or error
}

void interactive_mode() {
    printf("Welcome to Dr. Evil's shell!\n");
    while (1) {
        // maybe don't need this loop?
        write(STDOUT_FILENO, "mysh> ", 6);
        if(read_and_execute_commands(STDIN_FILENO, STDOUT_FILENO) == -1){
            // end of input stream or when exit command received
            break;
        }
    }
    printf("Exiting the shell. Goodbye!\n");
}

void batch_mode(char* script_path) {
    int input_fd; 
    int flag;
    if (script_path != NULL) {
        input_fd = open(script_path, O_RDONLY);
        if (input_fd == -1) {
            perror("open");
            exit(EXIT_FAILURE);
        }
    } else {
        input_fd = STDIN_FILENO;
    }
    // fix this part below
    flag = 0;
    while(flag != -1)
        {
            if (read_and_execute_commands(input_fd, STDOUT_FILENO) == -1){
            flag = -1;
            }
        }
    return;
}

int main(int argc, char *argv[]) {
    prev_command_status = 1;
    // set previous command status to 1 by default
    // assume that was successful
    if (argc == 1) {
        if (isatty(STDIN_FILENO)) {
            interactive_mode();
        } else {
            // batch mode w/h no provided file!
            batch_mode(NULL);
        }
    } else if (argc == 2) {
        // run batch mode with the script file
        batch_mode(argv[1]);
    } else {
        printf("Usage: %s [script_path]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}
