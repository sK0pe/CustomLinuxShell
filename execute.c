#include "mysh.h"
#include <sys/types.h>
#include <sys/wait.h>



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
				if(programID < 0){	// parent checks if fork() failed
					//  Fork has failed
					perror("fork");
					exitstatus = EXIT_FAILURE;
				}

				if(programID == 0){
					//  child process scope
					//  try and replace child with program
	//				if(execvp(t->argv[0], t->argv) < 0){
						//  if execute fails generate error
	//					perror(t->argv[0]);
						//  Exit the child as it couldn't be replaced with the
						//  program from the CMDTREE
	//					exit(EXIT_FAILURE);
	//				}
					//  exit(EXIT_FAILURE);
					execvp(t->argv[0], t->argv);
					//	If child still exists after exec, it has failed
					perror(t->argv[0]);
					exit(EXIT_FAILURE);
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

