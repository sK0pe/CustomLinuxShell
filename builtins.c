#include "mysh.h"
#include <sys/time.h>
#include <sys/param.h>

/*
	 CITS2002 Project 2 2015
	 Name(s):		Pradyumn Vij (, student-name2)
	 Student number(s):	21469477 (, student-number-2)
Date:		date-of-submission
*/

// -------------------------------------------------------------------
//	Commands run by the mysh shell program, internal commands
// -------------------------------------------------------------------


/*
 * 	mysh_cd
 * 	input:
 * 	pointer to array of character arrays
 * 	returns void
 * 	Attempts to change directory to the first
 * 	argument of input array.
 * 	Defaults to HOME directory.
 * 	If no '/' found, tries CDPATH
 *
 */
void mysh_cd(char **directory){
	if(directory[0] == NULL){	//  If arg not present
		chdir(HOME);	//  Default cd to home directory
	}
	else{
		if(chdir(directory[0]) != 0){	// If CD fails
			// If no forward slash in directory, consider CDPATH
			if(strchr(directory[0], '/') == NULL){
				char tempCDPATH[strlen(CDPATH)+1];
				strcpy(tempCDPATH, CDPATH);
				char tryPath[MAXPATHLEN];
				char *token = strtok(tempCDPATH, ":");
				//	Loop through possible paths from CDPATH
				while(token != NULL){
					strcpy(tryPath, token);
					strcat(tryPath, "/");
					strcat(tryPath, directory[0]);
					strcat(tryPath, "/");
					//	If directory found, exit
					if(chdir(tryPath) == 0){
						return;
					}
					//	clear path buffer
					memset(tryPath, 0, sizeof tryPath);
					token = strtok(NULL,":");
				}
			}
			perror("mysh_cd");
		}
	}
}

/*
 *	mysh_time
 *	
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
