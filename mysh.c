#include "mysh.h"

/*
   CITS2002 Project 2 2015
   Name:             Pradyumn Vij
   Student number:   21469477
   Date:                date-of-submission
 */

//  Local file global, protected
static int exitstatus;

/*
 *  getPriorExitStatus
 *  
 *  input: void
 *  return: integer representing the current
 *  value of static integer exitstatus.
 *  Simple getter function for exit status.
 *  Allows for exit from shell to be placed in 
 *  a function outside of main.
 */
int getPriorExitStatus(){
	return exitstatus;
}

/*
 *  Main
 *
 *  Executes from OS call.
 *  Initialises shell internal variables variables and
 *  runs the main shell loop for receiving input from
 *  user.  Parses, executes and frees input.
 */
int main(int argc, char *argv[]){
	//  REMEMBER THE PROGRAM'S NAME (TO REPORT ANY LATER ERROR MESSAGES)
	argv0	= (argv0 = strrchr(argv[0],'/')) ? argv0+1 : argv[0];
	argc--;				// skip 1st command-line argument
	argv++;

	//  INITIALIZE THE THREE INTERNAL VARIABLES
	char	*p;
	p		= getenv("HOME");
	HOME	= strdup(p == NULL ? DEFAULT_HOME : p);
	check_allocation(HOME);

	p		= getenv("PATH");
	PATH	= strdup(p == NULL ? DEFAULT_PATH : p);
	check_allocation(PATH);

	p		= getenv("CDPATH");
	CDPATH	= strdup(p == NULL ? DEFAULT_CDPATH : p);
	check_allocation(CDPATH);

	//  DETERMINE IF THIS SHELL IS INTERACTIVE
	interactive		= (isatty(fileno(stdin)) && isatty(fileno(stdout)));

	exitstatus = EXIT_SUCCESS;

	//  READ AND EXECUTE COMMANDS FROM stdin UNTIL IT IS CLOSED (with control-D)
	while(!feof(stdin)) {
		CMDTREE	*t = parse_cmdtree(stdin);
		if(t != NULL) {

			exitstatus = execute_cmdtree(t);
			printf("\nexit status = %d\n", exitstatus);
				
			free_cmdtree(t);
		}
	}
	if(interactive){
		fputc('\n', stdout);
	}
	return exitstatus;
}
