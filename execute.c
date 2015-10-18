#include "mysh.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/param.h>

/*
   CITS2002 Project 2 2015
   Name(s):		Pradyumn Vij (, student-name2)
   Student number(s):	21469477 (, student-number-2)
   Date:		date-of-submission
 */

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


//  THIS FUNCTION SHOULD TRAVERSE THE COMMAND-TREE and EXECUTE THE COMMANDS
//  THAT IT HOLDS, RETURNING THE APPROPRIATE EXIT-STATUS.
//  READ print_cmdtree0() IN globals.c TO SEE HOW TO TRAVERSE THE COMMAND-TREE

int execute_cmdtree(CMDTREE *t){
	int exitstatus;
	//	If CMDTREE is null return failure.
	if(t == NULL){
		return(EXIT_FAILURE);
	}else{
		exitstatus = EXIT_SUCCESS;
	}

	// If timing task
	if(strcmp(t->argv[0], "time") == 0){
		return mysh_time(t);	//  Pass memory address for 2nd argument
	}// If changing directory
	if(strcmp(t->argv[0], "cd") == 0){
		mysh_cd(&t->argv[1]);	//  Pass memory address for 2nd argument
	}
	else{
		switch(t->type){
			case N_COMMAND:{
				int childStatus;  //  used by wait, to check on child process
				pid_t waitID;	//  used by wait to check on child exit
				// Fork program to try to make a child process
				pid_t programID = fork();

				if(programID == 0){
					//  child process scope
					//  try and replace child with program
					if(execvp(t->argv[0], t->argv) < 0){
						//  if execute fails generate error
						perror(t->argv[0]);
						//  Exit the child as it couldn't be replaced with the
						//  program from the CMDTREE
						exit(EXIT_FAILURE);
					}
					//  exit(EXIT_FAILURE);
				}
				else if(programID < 0){	// parent checks if fork() failed
					//  Fork has failed
					perror("fork");
					exitstatus = EXIT_FAILURE;
				}
				else{	
					// parent process scope 
					//  waiting for child to end
					do{
						waitID = waitpid(programID, &childStatus, WUNTRACED);
						if(waitID == -1){	//	Check if waitpid exited with error
							perror("waitpid");
							exit(EXIT_FAILURE);
						}
						if(WIFEXITED(childStatus)){	//	If child exited
							//  Assign child exit status to output
							exitstatus = WEXITSTATUS(childStatus);
						}
					}
					//  Wait until child exits or is signalled to exit
					while(!WIFEXITED(childStatus) && !WIFSIGNALED(childStatus));
				}
				break;
			}
			default :
				fprintf(stderr,"%s: invalid NODETYPE in print_cmdtree0()\n",argv0);
				exitstatus = EXIT_FAILURE;
				break;
		}
	}
  return exitstatus;
}

