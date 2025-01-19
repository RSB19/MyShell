CC = gcc
CFLAGS = -g -Wall -std=c99 -fsanitize=address -O2
RM = rm -f

MYSHELL = mysh
DEBUGMYSHELL = dmysh

OBJ = myshell.o
OBJ2 = dmyshell.o

$(MYSHELL): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(MYSHELL)

$(DEBUGMYSHELL): $(OBJ2)
	$(CC) $(CFLAGS) -DDEBUG=1 $(OBJ2) -o $(DEBUGMYSHELL)

myshell.o: myshell.c myshell.h
	$(CC) -c myshell.c

dmyshell.o: myshell.c myshell.h
	$(CC) -c -DDEBUG=1 myshell.c -o dmyshell.o

all: $(MYSHELL) $(DEBUGMYSHELL)

clean:
	$(RM) $(MYSHELL) $(DEBUGMYSHELL) $(OBJ) $(OBJ2)
