#include "mysh.h"
#include <sys/time.h>

/*
	CITS2002 Project 2 2015
	Name(s):		Pradyumn Vij
	Student number(s):	21469477
	Date:		date-of-submission
*/

// -------------------------------------------------------------------
//	Commands run by the mysh shell program, internal commands
// -------------------------------------------------------------------


/*
 *  mysh_exit
 *  
 *  input: CMDTREE pointer
 *  returns: void
 *  Tasked with exiting the prior exit status if called with
 *  no arguments.
 *  Or if called with arguments, exits with the numeric value
 *  of the first argument after the call to exit
 */

void mysh_exit(CMDTREE *t){
	//  More than one argument:
	if(t->argc > 1){
		exit(atoi(t->argv[1]));
	}
	//  No arguments
	else{
		exit(getPriorExitStatus());
	}
}



/*
 * 	mysh_cd
 *  
 * 	input: pointer to array of character arrays
 * 	returns: integer
 * 	Attempts to change directory to the first
 * 	argument of input array.
 * 	Defaults to HOME directory.
 * 	If no '/' found, tries CDPATH
 *  Returns if chage directory successful.
 */
int mysh_cd(char **directory){
	int exitstatus;
	if(directory[0] == NULL){	//  If arg not present
		chdir(HOME);	//  Default cd to home directory
		exitstatus = EXIT_SUCCESS;
	}
	else{
		if(chdir(directory[0]) != 0){	
			// If CD fails
			// If no forward slash in directory, consider CDPATH
			if(strchr(directory[0], '/') == NULL){
				char tempCDPATH[strlen(CDPATH)+1];
				strcpy(tempCDPATH, CDPATH);
				char tryPath[MAXPATHLEN];
				char *token = strtok(tempCDPATH, ":");
				//	Loop through possible paths from CDPATH
				while(token != NULL){
					sprintf(tryPath, "%s/%s/", token, directory[0]);
					//	If directory found, exit
					if(chdir(tryPath) == 0){
						// Found path, exit successfully
						return EXIT_SUCCESS;
					}
					//	clear path buffer
					memset(tryPath, 0, sizeof tryPath);
					token = strtok(NULL,":");
				}
			}
			perror("mysh_cd");
			exitstatus = EXIT_FAILURE;
		}
		else{
			exitstatus = EXIT_SUCCESS;
		}
	}
	return exitstatus;
}

/*
 *	mysh_time
 *  
 *	input: CMDTREE pointer
 *	return: exit status of remainder of line
 *	Times all tasks that are run after the argument
 *	of time.
 *	Changes the structure of CMDTREE to reflect the
 *	removal of the first argument then reverts the changes.
 *	
 */
int mysh_time(CMDTREE *timedTree){
	if(strcmp(timedTree->argv[1],"exit") == 0 || timedTree->argv[1] == NULL){
		fprintf(stderr, "Timing error: No task to time.\n");
		return EXIT_FAILURE;
	}
	int timedStatus;
	struct timeval start, stop;
	//	Strip time from argument vector
	//	1 less arg count
	timedTree->argc--;
	//	argv[1] is now argv[0]
	char **temp = &timedTree->argv[0];
	timedTree->argv = &timedTree->argv[1];
	//  Start timing
	gettimeofday(&start, NULL);
	timedStatus = execute_cmdtree(timedTree);
	gettimeofday(&stop, NULL);
	//  Print time between start and stop in milliseconds, converted
	//  from microseconds
	fprintf(stderr, "%s completed in %fmsec\n", timedTree->argv[0],
			(double)(stop.tv_usec - start.tv_usec)/1000 
			+ (double)(stop.tv_sec - start.tv_sec)*1000);
	//	revert changes to CMDTREE
	timedTree->argc++;
	timedTree->argv = &(*temp);
	return timedStatus;
}
