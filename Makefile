#	Make file for mysh shell program
PROJECT = mysh
HEADERS = mysh.h
OBJ = mysh.o globals.o execute.o parser.o

C99 = cc -std=c99
CFLAGS = -Wall -pedantic -Werror


$(PROJECT) : $(OBJ)
	$(C99) $(CFLAGS) -o $(PROJECT) $(OBJ)

mysh.o : mysh.c
	$(C99) $(CFLAGS) -c mysh.c

globals.o : globals.c $(HEADERS)
	$(C99) $(CFLAGS) -c globals.c

execute.o : execute.c $(HEADERS)
	$(C99) $(CFLAGS) -c execute.c

parser.o : parser.c $(HEADERS)
	$(C99) $(CFLAGS) -c parser.c

clean:
	rm -f $(PROJECT) $(OBJ)
