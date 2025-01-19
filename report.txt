=====================
Authors:

Rohit Bellam  - rsb204
Vatsal Malkari - vm593

=====================

** NOTE: At the end of this file is instructions on how to use the Makefile! **

Overview:

The aim of this project is to make a program that will mimic a command-line shell. In this case, we have 
designed and implemented a simple command-line shell, similar to bash or zsh. Here, our program mysh will 
have TWO main modes of operation - interactive mode and batch mode. Both of these modes will read and attempt
to interpret a sequence of commands. We then take this sequence of commands, break it up into individual
tokens, and utilize our command-parsing algorithm to determine how to properly run the command. For interactive
mode, we only receive ONE line which will also include a welcome and goodbye message. As for batch mode, it 
will read commands PER line from a script file (ends in .sh) and execute the commands in "batch", hence the 
name batch mode. Specifically, the main program myshell.c PAIRED with myshell.h, has multiple function prototypes 
defined along with our chosen data structure will be discussed more in depth later on.

Now, our implementation uses a TokenList, which is essentially an ArrayList data structure. Now, we decided to 
rename it to TokenList using typedef for readibility purposes. Included in our TokenList is a char ** array, 
which is essentially a pointer to an array of strings. Compared to a tries or a hash table, this is not only the 
most simple and logical way to keep track of all of our tokens, BUT this simplistic approach allows us to properly
step through our token stream and figure out how to run the command.Now, one NOTE is that we are using ONE global 
variable called int prev_command_status. The reason we need this global variable is to handle the conditionals 
requirement of this project, and having a global variable allows our program, mysh, to keep track of the previous 
command's exit status. 

To recap, we are using the struct which we have defined as the primary way to store the tokens and use those tokens
along with other information in our TokenList struct such as int count and int size, which tell us HOW many tokens
are actually in our TokenList and the maximum size of our TokenList. Keep in mind that we do dynamically allocate 
using malloc and realloc so that way we can adjust the size if we reach a certain threshold. 

Detailed below are what is included in our project, details on each file, why we chose certain approaches,
and our test cases/strategy.

The project is made up of the following:

    1. myshell.c: this file has ALL of our main logic behind the process of effectively creating a "simple"
                  commmand-line shell with error checking and other vital features. Mysh includes two modes-
                  interactive mode and batch mode, and both of these read input coming from either a provided
                  script file OR from the user typing inputs. It also includes our main method which handles    
                  how to properly decide running in batch mode or interactive mode. 

    2. myshell.h: this header file includes function prototypes, macro definitions such as DEBUG, and our struct
                  called TokenList.

    3. mywords.txt: a text fle that includes all pokemon from generation 8. This will primarly be used to 
                    test with mysh to see if our shell can work with .txt files, as well as other file formats.

    4. testcases.txt: a txt file that includes in detail all of our test cases to handle various examples
			  of how we would run ./mysh. This will be described later in the README.txt

	5. Makefile: a recipe file which includes all the proper implementation to run "make clean" and then
		     "make all" to automate testing. Details on how to use the makefile are further below.

	6. myscript.sh: a script file which includes various commands to RUN and test our implementation of mysh. 
					Note: Make sure to use this with batch mode, NOT with interactive mode. If you want, you 
					can boot up interactive mode by doing ./mysh and type each command one by one to see the 
					result. 

	7. extras: a directory which has sub-directories AND files numbered below:

        a) extras/test1file.txt, test2file.txt, test1.c, test2.c   - 4 total files
		b) extras/subextras/test1.c, test2.c, file1.txt file2.txt  - 4 total files  

        NOTE: A total of 8 files! 
    
    Details on what is included in all of these files will be explained later below!

-------------------------

Structure/Vision:

After planning out what data structure to use, we utilized a ArrayList to store all of the tokens, which 
are basically char * storing each of our tokens. Now, this is via a typedef struct called TokenList, creating 
a data structure which we can use to store all of the tokens that we encounter throughout the token steam. That 
is, we used a char ** array, which in this case, is a pointer to an array of strings. Notice that our TokenList
is a sequence of non-whitespace characters, except for <, >, and |, which are always individual tokens on their
own.

Now, the main reason we went with an char ** array to store our tokens is because of the following:

a) an array allows us to manage all of tokens, allowing us to step through our tokens easily.
b) we can easily traverse an array relatively quickly and thus go through our token steam, helping us 
to determine the argument list and whether the command includes redirection, pipes/pipelines, etc depending
on the command being executed
c) dynamically allocate tokens whether we are in batch mode or interactive mode to allow for expansion of our 
preserved size using malloc and realloc. 

The array was implemented with the following:

	- char ** tokens
	- int count
	- int size

Here, in our struct, we have 3 objects: a char ** pointer, the int count, AND the int size. Here, tokens is a 
char ** pointer that stores our tokens, which can include the following:
	a) built in commands (and arguments) 
		e.g: which, cd, pwd, and exit
	b) non-built in commands (and arguments)
		e.g: ls, grep, sort, mkdir, etc.
	c) executable files (and arguments)
		e.g: ./myprogram, etc.
	d) Special tokens:
		e.g: *, <, >, |

========================
Files:
======================== 

myshell.c
-------------------------


The first file, myshell.c contains our data structure (which is a struct) that stores the tokens that we encounter
either in batch mode or interactive mode. Notice that these tokens are a sequence of non-whitespace characters, 
except for <, >, and |, which are always individual tokens on their own. We are also storing int count, which is 
the number of tokens that we are storing, and int size which keeps track of the relative size of our TokenList. 
Note that the size is dynamically adjusted without any need for manual input. There are also the various functions 
which will be detailed further. Here, our TokenList is composed of the following:

a) Storing the token words in our struct called TokenList which contains:
	char ** tokens - a pointer to an array of token strings
	int size - total size of our TokenList
	int count - total number of tokens stored in TokenList

b) Traversing through all arguments provided (can be either files, directories, built-in commands, etc) and running
	commands in proper order.

Functions:

	void tokenize_input(char* input, int bytes, TokenList* tokenList);
	
	tokenize_input takes three arguments: the char * input which will be our buffer coming from either batch
	mode or interactive mode, the int bytes which indicates the number of bytes we are reading, and the data
	structure TokenList which is storing all of the tokens. 

	First, we initialize our TokenList with a helper function called init_token_list to start using the 
	TokenList. We use malloc to dynamically allocate memory on the heap. 

	Next, we have a loop that traverses through our buffer input and splits to form a token(word) if it 
	encounters either a whitespace OR null terminator. If it encounters either of these, it then stores 
	the token built into our TokenList. Now, the key implementation if we run out of space in our
	TokenList, in which case we call realloc on our TokenList to get more space. This helps us to 
	scale our TokenList if we encounter more tokens. 

	Now, if we find a char that is either <, >, or |, then we store those as individual tokens themselves for
	helping with our command parsing algorithm. 

	Next, there is one final case where if we encounter a special token as the last character of our command 
	that we have read from our buffer, then we simply just ignore it.  

	void init_token_list(TokenList* tokenList);

	init_token_list takes one argument: our TokenList data structure which holds our tokens and additional
	information. 

	The function is setting the count to 0, size initially to 10, and then uses malloc to allocate our 
	char ** tokens array with error checking in place. 

	int matches_pattern(char* filename, char* prefix,  char* suffix);

	matches_pattern is a utility function paired with match_wildcards() that takes in 3 arguments: the 
	filename which we have to match, a prefix and a suffix. Notice that all of these are char *

	Essentially, this function is first using the length of each of these 3 values, and then doing
	a couple of conditional statements to check if the filename is at least as long as the prefix + suffix.
	Next, we check if the filename starts with the prefix and also if the filename ends with the suffix. 
	
	void handle_wildcards(TokenList* tokenList);

	handle_wildcards takes one argument: our TokenList data structure which holds our tokens and additional
	information. 

	This function processes a list of tokens, each potentially containing a wildcard character (''), to find 
	and replace them with actual file or directory names matching the wildcard pattern within a specified or 
	current directory. It searches for the last occurrence of a slash ('/') in each token to separate the 
	directory path from the wildcard pattern. Using the directory path (or the current directory if none is 
	specified), it then scans for files or directories matching the pattern, where the wildcard '' is treated 
	to match any sequence of characters. Found matches replace the original token, and if multiple matches 
	are found, they are inserted into the token list, expanding it as needed. The function handles memory 
	allocation dynamically to accommodate the potentially changing size of the token list.

	void setup_redirection(int* input_fd, int* output_fd);

	setup_redirection takes two arguments: the input file descriptor and the output file descriptor, both 
	of which are int *. 

	This function sets up input and output redirection for a process. If the specified input file descriptor 
	(input_fd) is different from the standard input (STDIN_FILENO), it duplicates input_fd to STDIN_FILENO 
	and closes the original input_fd. Similarly, if the specified output file descriptor (output_fd) is not 
	the standard output (STDOUT_FILENO), it duplicates output_fd to STDOUT_FILENO and then closes the original 
	output_fd. This ensures that the process's standard input and output are redirected to the specified 
	file descriptors.

	int is_builtin_command(char* command);

	is_builtin_command is a helper function to execute_builtin_command() and takes in one argument: a command 
	token value which is a char *.

	This function just takes the token command and checks using strcmp() whether it is any one of the 4
	built in functions: exit, which, pwd, and cd.

	void execute_builtin_command(char* args[], int* should_exit);

	execute_builtin_command takes two arguments: the tokens arguments which is a char ** array, and the flag 
	should_exit which is an int * to determine if we have to exit the program or not.

	This function interprets and executes a set of built-in commands for a shell-like interface, including 
	"exit", "cd" (change directory), "pwd" (print working directory), and "which". Upon receiving "exit", 
	it sets a flag to terminate the shell session and prints all arguments passed with the "exit" command 
	before displaying a goodbye message. For "cd", "pwd", and "which", it attempts to execute the respective 
	actions, setting prev_command_status to 1 on success and 0 on failure. If an unknown command is provided, 
	it prints an error message and sets prev_command_status to 0.

	void print_tokens(TokenList* tokenList);

	print_tokens takes one argument: our TokenList data structure which holds our tokens and additional
	information. 

	This function simply prints our TokenList and this is paired with turning on DEBUG mode to 1 for myshell
	to help analyze if the token stream is correct or not.

	void tokenize_file_input(char* line, int bytes, TokenList* tokenList);

	tokenize_file_input takes three arguments: the line of bytes to read, the number of bytes, and our TokenList 
	data structure which holds our tokens and additional information. 

	This function simply uses the tokenize_input() function described earlier to handle working in batch mode, 
	where it reads the entire contents of a script file, launching/running commands, etc.

	void free_token_list(TokenList* tokenList);

	free_token_list takes in one argument: our TokenList data structure which holds our tokens and additional
	information. 

	This function takes our TokenList and frees our data structure. Since we allocated using malloc, we have to 
	call free on EACH individual object. It also resets the other fields in our struct. 

	void execute_single_command(char* args[]);

	execute_single_command takes in one argument: the char * arguments which contains our tokens to run the 
	respective command. 

	This function attempts to execute a command provided through its arguments array. Initially, it checks if 
	a command is provided and whether the command includes a directory path, attempting to execute it directly 
	if so and setting `prev_command_status` accordingly based on the executable's presence and permissions. 
	If the command does not include a directory path, the function searches for the executable in a predefined 
	set of directories (`/usr/local/bin`, `/usr/bin`, `/bin`). If found, it executes the command using `execv`, 
	setting `prev_command_status` to indicate success; if not, it prints an error message. The function uses 
	`stat` to check for the existence and executability of the command and terminates the process with an 
	error if execution fails or the command cannot be found.

	int change_directory(char* path);

	change_directory takes in one argument: the path of the char * token to change directory. 

	This function checks to make sure that the path isn't NULL to perform error checking and then 
	uses chdir() with the path provided to "attempt" with changing the directory and it returns -1,
	then we handle error checking.

	int print_working_directory();

	print_working_directory takes in no arguments and prints out the working directory. 

	It uses getcwd() to grab the current working directory. If it is NULL, it handles error checking and 
	otherwise prints out the current working directory.

	int execute_which(char* program_name);

	execute_which takes in one argument: the program name to run a search with which. 

	This function implements a simplified version of the `which` command, used to locate a program file in 
	the user's path. It searches for the specified program name in a predefined set of directories 
	(`/usr/local/bin`, `/usr/bin`, `/bin`). If the program is found in any of these directories, it prints 
	the full path to the program and returns 1 to indicate success. If the program cannot be found or if no 
	program name is provided as input, it prints an error message and returns 0, indicating failure. 
	The existence and executable status of the program are verified using the `stat` function.

	int execute_command(TokenList* tokens);

	execute_command takes in one argument: our TokenList data structure which holds our tokens and additional
	information. 

	This function executes a command represented by a `TokenList` structure, handling input/output redirection, 
	pipes, and determining if a command is built-in or external. It first processes the tokens to filter out 
	and handle input and output redirection by opening file descriptors as needed. If the command includes a 
	pipe, it creates a pipe and forks two processes: the first executes the command before the pipe with its 
	output redirected to the pipe, and the second executes the command after the pipe, potentially using the 
	pipe's output as input. For commands without a pipe, it determines if the command is built-in and executes 
	it directly, applying any necessary redirection. External commands are executed by forking a child process, 
	setting up any required redirection, and using `execv` to run the command.

	The function also handles cleanup and error checking, closing file descriptors and freeing allocated memory 
	as necessary. It maintains a `prev_command_status` flag to track the execution status of the last command 
	and sets a `should_exit` flag if an exit condition is met, such as an explicit exit command or an error. If 
	a pipe is used, it waits for both child processes to finish and checks their exit statuses to determine if 
	the command executed successfully. For non-piped commands, it waits for the child process to complete and 
	checks its exit status. The function exits the shell if the `should_exit` flag is set, indicating that an 
	exit command was issued or a critical error occurred.

	void remove_first_token(TokenList* tokenList);

	remove_first_token takes in one argument: our TokenList data structure which holds our tokens and additional
	information. 

	This function takes in our TokenList data structure and first checks to make sure that our tokenlist is 
	NOT empty. It then frees the memory of the FIRST token and then shifts all of our allocated objects by 
	1 to the left.

	int process_and_execute_command(TokenList* tokenList);

	process_and_execute_command takes in one argument: our TokenList data structure which holds our tokens and 
	additional information. 

	This function processes and executes commands from a `TokenList` structure, incorporating wildcard expansion, 
	conditional execution based on the outcome of previous commands, and special handling for built-in commands 
	like 'exit', 'cd', 'which', and 'pwd'. It starts by expanding any wildcards in the tokens. It then checks if 
	the first token is 'then' or 'else', setting the `executeCommand` flag based on the `prev_command_status` to 
	determine whether the command should proceed. If execution is permitted, it checks for special commands and 
	handles them accordingly, including a specific exit process for the 'exit' command. Debug mode triggers printing 
	of the tokens for diagnostics. After executing a command, it updates `prev_command_status` with the execution 
	result, cleans up by freeing the token list, and returns 1 to indicate the process completion. Commands are 
	skipped based on conditional logic related to the previous command's success or failure, streamlining execution 
	flow in a script-like fashion.

	int read_and_execute_commands(int input_fd, int output_fd);

	read_and_execute_commands takes in two arguments: the input file descriptor and the output file descriptor.

	This function reads and executes commands from a given input file descriptor (`input_fd`), managing both interactive 
	and batch modes based on the input source. It dynamically allocates and expands a buffer (`line`) to store input 
	commands, doubling its size whenever the capacity is nearly exceeded. In interactive mode (detected when `input_fd` 
	is standard input and attached to a terminal), it processes and executes commands line by line, immediately returning 
	after executing a single command. In batch mode, it reads the entire input into the buffer, processes, and executes 
	each command separated by newline characters, handling multiple commands in a single read operation. The function 
	uses tokenization to convert command strings into a structured format (`TokenList`) before executing them and manages 
	resources appropriately by closing file descriptors and freeing allocated memory. The return value indicates the success 
	or failure of command execution or signifies the end of input or an error condition. 

	void interactive_mode();

	interactive_mode takes in no arguments and determines when we are running myshell if we are in interactive mode or 
	batch mode. This is coupled with int main().

	This function prints a welcome message and then runs a loop to call read_and_execute_commands() UNTIL we either 
	end the input stream OR exit is called, in which we print out a goodbye message and exit the program.

	void batch_mode(char* script_path);

	interactive_mode takes in no arguments and determines when we are running myshell if we are in interactive mode or 
	batch mode. This is coupled with int main().

	This function attempts to open the file assuming that we are in batch mode OR if we are reading from STDIN_FILENO.
	If we are opening the script file, we call open() and then keep track of a flag while running a loop to call
	read_and_execute_commands() until we encounter end of file OR exit is called.

	Now, if we are reading from STDIN_FILENO, we still do the same approach as above but slightly different approach.

	int main(int argc, char* argv[]);

	main takes two arguments which we are going to use to keep track of the number of arguments AND what files we 
	are passing in, which in this case can only be a script file OR from the output of STDIN_FILENO (in this case,
	a terminal)

	If the number of arguments is one, we check to make sure that STDIN_FILENO is coming from a terminal by using
	isatty() and if this holds, we call interactive_mode(). Otherwise, we know that we are in batch mode with 
	no provided file.

	Now, if the number of arguments is JUST 2, then we run batch mode with the file, which in this case is our
	script file. 

	If none of these conditions match, we print an error message and exit the program.

--------------------

extras:

"extras" is a directory that includes the following:

	a) 2 children directories called subextras and subsubextras
	b) 8 children files called: 
		- test1file.txt, test2file.txt, test1.c, test2.c
		- test1.c, test2.c, file1.txt, file2.txt

Note: All of the .c files have the same printf message listed below in int main. Now, 
as for the txt files, they all have about 50 random words each, making for a total of
200 words.

Contents of the .c files: (All the same)

"#include <stdio.h>"
"#include <stdlib.h>"
"#include <string.h>"
"#include <dirent.h>"
"#include <sys/stat.h>"
"#include <ctype.h>"
"#include <unistd.h>"
"#include <fcntl.h>"


int main(int argc, char** argv){

printf("Hello! My name is Dr. Evil! I am an evil villan invading your project.\n");


return EXIT_SUCCESS;
}

--------------------

====================
Testing Plan/Strategy
====================

Before we made the necessary test case(s) to meet our requirements of what mysh.c should meet, we wanted to 
highlight our testing approach and strategy in order to give a better spotlight on how we "tested" our 
requirements and why we made the test cases/requirements. Outlined below are two parts:

a) "Requirements" - goes over key requirements we want our implementation to meet!

b) testcase.txt - all of the test cases (21 total) which are various commands that check these requirements!


Requirements:

1. mysh properly tokenizes from our buffer reading either from batch mode or interactive mode.

   This is an important step as we want to ensure that our program can handle an unknown amount of bytes
   to read from and extracts the tokens properly.

2. mysh handles redirection, pipes (only one), and wildcard tokens effectively, allowing the command parsing
   algorithm to properly execute the command.

	This is especially crucial since the normal shell can handle these cases effectively and our shell
	should also do the same. It can work with commands, files, etc.

3. mysh correctly opens (in batch mode) the script file, runs all of the commands as if we ran them in interactive mode

   The reason we made this a requirement is to ensure that our program actually correctly reads each command PER line,
   runs the command, and continues with the rest of the commands.

4. mysh prints welcome and exiting messages (in interactive mode), reads one LINE of input to run the command, and continues
   to receive input UNTIL the end of input stream OR if exit is called.

   This is especially important as we want to make sure that interactive mode is actually "interactive" in that it 
   lets the user type in commands to run like how bash or zsh would.

5. mysh handles running built-in commands, non-built in commands, and programs to run

   We have to make sure that our code can properly run programs in our working directory, built-in commands such as
   which, cd, pwd, and exit, as well as non-built in commands like ls, cp, mkdir, etc.

   Keep in mind that we can also nest wildcards with files such as ls *.c to search for all matching files that end
   with .c 

6. mysh keeps redirection in precedence over a pipe

   Here, we want to make sure that if we have a pipe followed by a redirection, in which both impact either the STDIN_FILENO
   or the STDOUT_FILENO, redirection takes precedence. This means the data sent to the pipe is lost

Outlined below are the various test cases in our testcase.txt file that test all of these 7 key requirements:

------------------

testcases.txt:

testcases.txt includes 21 total test cases, which will help to test our 7 requirements above.
Below, instructions are given on how to run each test case and the output.

Since we didn't want to make our README.txt file too long and hard to read, please open the 
file "testcases.txt" to get more details about each test case and how to run it.

NOTE: All of these test cases are IN our script file called myscript.sh!

-----------------

myscript.sh

myscript.sh has most of our test cases to test and see if they work or not. Refer to 
testcases.txt to see the 18 test cases that we wrote for more detail. 

Contents of myscript.sh:

echo hello
pwd
cd extras
sort *.txt | grep ra
ls test*file.txt
cd subextras
sort *.txt | grep la
sort file1.txt | grep ra < file2.txt
pwd
cd fakedir
cd ../..
false
then echo should not print
true
then echo should print
false
else echo should print
true
else echo should not print
sort mywords.txt > result.txt
grep hi < mywords.txt
sort mywords.txt | grep ra
ls extras/test*file.txt
ls *.c *.txt
then exit whoops leaving now

Output:

hello

/common/home/rsb204/CS214/P3

bizrate
draft
monorail
pranks
programas
spraying

test1file.txt  test2file.txt

classe
habla
lastly
temporal
radon
tracking
brackets

/common/home/rsb204/CS214/P3/extras/subextras

cd: No such file or directory

should print
should print

Thievul
Pincurchin
Urshifu

Barraskewda
Cinderace
Copperajah
Cramorant
Dracovish
Dracozolt
Dragapult
Drakloak
Duraludon
Grapploct
Regidrago
Silicobra

extras/test1file.txt  extras/test2file.txt

myshell.c  mywords.txt  README.txt  result.txt  testcases.txt

whoops 
leaving 
now 
Exiting the shell. Goodbye!

Notice that this is correct!

-----------------

Makefile

We have created a makefile that will act as a recipe to automatically create all of the necessary AND valid 
executable file(s). Now, in order to USE this make file, you can do two things:

1. typing make will ONLY create mysh. It will not make the executable debugmysh

2. typing "make all" will create ALL of the executable files listed below:

a) mysh
b) dmysh

You can then type "make clean" to remove all the executable(s) and .o object files

Now, how do you run these files? Here is how you run the files:

./mysh - for interactive mode
./mysh myscript.sh - for batch mode with provided file
etc...

./dmysh - for interactive mode
./dmysh myscript.sh - for batch mode with provided file
etc...

dmysh turns on DEBUG mode and prints out the Tokens read PER line either in interactive mode
or in batch mode. This helps use to see if our algorithm is correctly parsing out the tokens
and determining how to run the commands.