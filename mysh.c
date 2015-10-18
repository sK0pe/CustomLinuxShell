#include "mysh.h"

/*
   CITS2002 Project 2 2015
   Name:             Pradyumn Vij
   Student number:   21469477
   Date:                date-of-submission
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

  int exitstatus	= EXIT_SUCCESS;

//  READ AND EXECUTE COMMANDS FROM stdin UNTIL IT IS CLOSED (with control-D)
  while(!feof(stdin)) {
		CMDTREE	*t = parse_cmdtree(stdin);
		if(t != NULL) {

//  WE COULD DISPLAY THE PARSED COMMAND-TREE, HERE, BY CALLING:
	    //print_cmdtree(t);
			//  Check if need to exit mysh
			if(strcmp(t->argv[0], "exit") == 0){ 
				if(t->argc > 1){ 
					//  Exit with arg, exit with numeric interpretation
					printf("access to 2nd argument, exit status = %d\n", atoi(argv[1]));
					exitstatus = atoi(argv[1]);					
				}
				//  Exit with no args, exit with last exitstatus
				printf("I'm exiting here with an exit status of %d\n", exitstatus);
				break;
			}
			printf("exitstatus before execution of %s is = %d\n",t->argv[0], exitstatus);
			exitstatus = execute_cmdtree(t);
			printf("exit status after execution of %s is = %d\n", t->argv[0], exitstatus);
			
			free_cmdtree(t);
		}
	}
	if(interactive){
		fputc('\n', stdout);
	}
	return exitstatus;
}
