#include "mysh.h"
#include <sys/types.h>
#include <sys/wait.h>

/*
   CITS2002 Project 2 2015
   Name(s):		Pradyumn Vij (, student-name2)
   Student number(s):	21469477 (, student-number-2)
   Date:		date-of-submission
 */

// -------------------------------------------------------------------


void mysh_cd(char **argv){
	if(argv[0] == NULL){	//  If arg not present
		chdir(HOME);
	}
	else{
		if(chdir(argv[0]) != 0){
			perror("mysh_cd");
		}
	}
}

//  THIS FUNCTION SHOULD TRAVERSE THE COMMAND-TREE and EXECUTE THE COMMANDS
//  THAT IT HOLDS, RETURNING THE APPROPRIATE EXIT-STATUS.
//  READ print_cmdtree0() IN globals.c TO SEE HOW TO TRAVERSE THE COMMAND-TREE

int execute_cmdtree(CMDTREE *t)
{
	int exitstatus;
	// If Trying to change directory
	if(strcmp(t->argv[0], "cd")==0){
		mysh_cd(&t->argv[1]);	//  argv[1] becomes argv[0] for mysh_cd
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
				}
				//  If fork succeeds
				if(programID == 0){	//  child process successfully initiated
					if(execvp(*t->argv, t->argv) < 0){	//  if execute fails
						//fprintf(stderr, "Execution of %s failed.\n", *t->argv);
						perror("mysh");
						exitstatus = EXIT_FAILURE;
					}
				}
				else{	// parent process (mysh) waiting for child to end
					while(wait(&childStatus) != programID);
				}
				break;
			}

			default :
				fprintf(stderr,"%s: invalid NODETYPE in print_cmdtree0()\n",argv0);
				exit(1);
				break;
		}
	}
	

  return exitstatus;
}
