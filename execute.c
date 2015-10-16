#include "mysh.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>

/*
   CITS2002 Project 2 2015
   Name(s):		Pradyumn Vij (, student-name2)
   Student number(s):	21469477 (, student-number-2)
   Date:		date-of-submission
 */

// -------------------------------------------------------------------


void mysh_cd(char **directory){
	if(directory[0] == NULL){	//  If arg not present
		chdir(HOME);
	}
	else{
		if(chdir(directory[0]) != 0){
			perror("mysh_cd");
		}
	}
}

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

int execute_cmdtree(CMDTREE *t)
{
	int exitstatus;
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
				pid_t programID;
				
				programID = fork();

				if(programID < 0){
					//fprintf(stderr, "Forking %s failed\n", *t->argv);
					perror("fork");
					exitstatus = EXIT_FAILURE;
					break;
				}
				//  If fork succeeds
				if(programID == 0){	//  child process successfully initiated
					if(execvp(t->argv[0], t->argv) < 0){	//  if execute fails
						fprintf(stderr, "Execution of %s failed.\n", *t->argv);
						perror("mysh");
					}
					exitstatus = EXIT_FAILURE;
				}
				else{	// parent process (mysh) waiting for child to end
					while(wait(&childStatus) != programID);
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
