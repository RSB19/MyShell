# MyShell

## **Authors**
Rohit Bellam  - rsb204\
Vatsal Malkari - vm593

NOTE: At the end of this file is instructions on how to use the Makefile! If you would like more detailed information about the project implementation, please navigate to the report.txt file below:

[Report](https://github.com/RSB19/MyShell/blob/main/report.txt)

## Overview

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

     1. myshell.c: This file has ALL of our main logic behind the process of effectively creating a "simple"
                  commmand-line shell with error checking and other vital features. Mysh includes two modes-
                  interactive mode and batch mode, and both of these read input coming from either a provided
                  script file OR from the user typing inputs. It also includes our main method which handles    
                  how to properly decide running in batch mode or interactive mode. 

     2. myshell.h: This header file includes function prototypes, macro definitions such as DEBUG, and our struct
                  called TokenList.

     3. mywords.txt: A text fle that includes all pokemon from generation 8. This will primarly be used to 
                    test with mysh to see if our shell can work with .txt files, as well as other file formats.

     4. testcases.txt: A text file that includes in detail all of our test cases to handle various examples
		       of how we would run ./mysh. This will be described later in the README.txt

     5. Makefile: A recipe file which includes all the proper implementation to run "make clean" and then
		  "make all" to automate testing. Details on how to use the makefile are further below.

     6. myscript.sh: A script file which includes various commands to RUN and test our implementation of mysh. 
		     Note: Make sure to use this with batch mode, NOT with interactive mode. If you want, you 
		     can boot up interactive mode by doing ./mysh and type each command one by one to see the result. 

     7. extras: a directory which has sub-directories AND files numbered below:

        a) extras/test1file.txt, test2file.txt, test1.c, test2.c   - 4 total files
	    b) extras/subextras/test1.c, test2.c, file1.txt file2.txt  - 4 total files  

NOTE: A total of 8 files! 
Details on what is included in all of these files will be explained later below!

## Structure/Vision

After planning out what data structure to use, we utilized a ArrayList to store all of the tokens, which 
are basically char * storing each of our tokens. Now, this is via a typedef struct called TokenList, creating 
a data structure which we can use to store all of the tokens that we encounter throughout the token steam. That 
is, we used a char ** array, which in this case, is a pointer to an array of strings. Notice that our TokenList
is a sequence of non-whitespace characters, except for <, >, and |, which are always individual tokens on their
own.

Now, the main reason we went with an char ** array to store our tokens is because of the following:

a) an array allows us to manage all of tokens, allowing us to step through our tokens easily.\
b) we can easily traverse an array relatively quickly and thus go through our token steam, helping us 
to determine the argument list and whether the command includes redirection, pipes/pipelines, etc depending
on the command being executed\
c) dynamically allocate tokens whether we are in batch mode or interactive mode to allow for expansion of our 
preserved size using malloc and realloc.

The array was implemented with the following:

	- char ** tokens
	- int count
	- int size

Here, in our struct, we have 3 variables: a char ** pointer, the int count, AND the int size. 

Now, with the tokens variable itsemf, its the char ** pointer that stores the tokens, which includes:

	a) built in commands (and arguments)
		e.g: which, cd, pwd, and exit
	b) non-built in commands (and arguments)
		e.g: ls, grep, sort, mkdir, etc.
	c) executable files (and arguments)
		e.g: ./myprogram, etc.
	d) Special tokens:
		e.g: *, <, >, |

## Makefile

We have created a makefile that will act as a recipe to automatically create all of the necessary AND valid 
executable file(s). Now, in order to USE this Makefile, you can do two things:

1. typing make will ONLY create mysh. It will not make the executable debugmysh

2. typing "make all" will create ALL of the executable files listed below:

	a) mysh\
	b) dmysh

You can then type "make clean" to remove all the executable(s), object files, and other types created by the compiler

Now, how do you run these files? Here are the options:

```
./mysh - for interactive mode
./mysh myscript.sh - for batch mode with provided files


./dmysh - for interactive mode
./dmysh myscript.sh - for batch mode with provided files
```

dmysh turns on DEBUG mode and prints out the Tokens read PER line either in interactive mode
or in batch mode. This helps use to see if our implementation is correctly parsing out the tokens
and determining how to run the commands.

## Instructions

In order to run the myShell implementation, please type or copy and paste the following commands into the terminal after downloading the tar file:

```
tar -xf your-p3.tar
cd P3
make
```

Remember, you can also type `make all` to create the debug myShell if you would like for debugging.

If you don't want to use the tar, you can just use the files present in this repository and run the shell below:

```
./mysh <commands/file>
```

Here, you can either run myShell in batch mode or interactive mode. Note that for interactive mode, you have to only type:

```
./mysh
```

And for batch mode, you can pass in the relevant files and/or commands to execute them all at once!






Copyright ⓒ Rohit Bellam
