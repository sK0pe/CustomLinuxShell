#include "mysh.h"
#include <errno.h>

/*
	CITS2002 Project 2 2015
	Name:			Pradyumn Vij
	Student number:	21469477
	Date:			26/10/2015
 */

//  Local file global, protected
static int exitStatus;

/*
 *  getPriorExitStatus
 *  
 *  input: void
 *  return: integer
 *
 *  Getter for static exit status.
 *  
 */
int getPriorExitStatus(){
	return exitStatus;
}

/*
 *  clean_background
 *
 *  input: void
 *  return: void
 *
 *  Helper for main function, provides a wait function
 *  to catch any zombie background functions which have
 *  finished.
 */
void clean_background(void){
	pid_t zombieID;
	while(zombieID > 0){
		//  WNOHANG returns immediately, pid of -1 grabs all processes
		zombieID = waitpid(-1, NULL, WNOHANG);
	}
	if(zombieID < 0 && errno != ECHILD){
		perror("waitpid backround");
	}
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
	argv0	= (argv0 = strrchr(argv[0], '/')) ? argv0+1 : argv[0];
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

	exitStatus = EXIT_SUCCESS;

	//  READ AND EXECUTE COMMANDS FROM stdin UNTIL IT IS CLOSED (with control-D)
	while(!feof(stdin)) {
		CMDTREE	*t = parse_cmdtree(stdin);
		if(t != NULL) {
			exitStatus = execute_cmdtree(t);
			free_cmdtree(t);
		}
		//  Cleanup any zombie processes
		clean_background();
	}

	if(interactive){
		fputc('\n', stdout);
	}
	return exitStatus;
}
