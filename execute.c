#include "mysh.h"
#include <sys/types.h>
#include <sys/wait.h>

int launch_command(CMDTREE *t){
	print_cmdtree(t);
	int launchStatus;
	int childStatus;  // used by wait, to check on child process
	pid_t waitID;	// used by wait to check on child exit
	
	// Fork program to try to make a child process
	pid_t programID = fork();
	if(programID < 0){	// parent checks if fork() failed
		//  Fork has failed
		perror("fork");
		return EXIT_FAILURE;
	}

	if(programID == 0){
		//  child process scope
		//  try and replace child with program
		execvp(t->argv[0], t->argv);
		//  If child still exists after exec, execution failed
		perror(t->argv[0]);
		exit(EXIT_FAILURE);
	}
	else{	
		//  parent process scope 
		do{
			//  Make parent wait for child to end
			waitID = waitpid(programID, &childStatus, WUNTRACED);
			//  Check if wait exited with error
			if(waitID == -1){
				perror("waitpid");
				exit(EXIT_FAILURE);
			}
			//  Check if child exited
			if(WIFEXITED(childStatus)){
				//  Assign child exit status to output
				launchStatus = WEXITSTATUS(childStatus);
			}
		}
		//  Wait until child exits or is signalled to exit
		while(!WIFEXITED(childStatus) && !WIFSIGNALED(childStatus));
	}
	return launchStatus;
}

//  THIS FUNCTION SHOULD TRAVERSE THE COMMAND-TREE and EXECUTE THE COMMANDS
//  THAT IT HOLDS, RETURNING THE APPROPRIATE EXIT-STATUS.
//  READ print_cmdtree0() IN globals.c TO SEE HOW TO TRAVERSE THE COMMAND-TREE

int execute_cmdtree(CMDTREE *t){
	print_cmdtree(t);
	int exitstatus;
	//	If CMDTREE is null return failure.
	if(t == NULL){
		return(EXIT_FAILURE);
	}else{
		exitstatus = EXIT_SUCCESS;
	}
	//  Check type of Command Branch
	switch(t->type){
		case N_COMMAND:{
			//  Check for builtin tasks
			//  Exit task
			if(strcmp(t->argv[0], "exit") == 0){
				if(t->argc > 1){
					exit(atoi(t->argv[1]));
				}
				else{
					exit(getPriorExitStatus());
				}
			}
			//  Timing task
			if(strcmp(t->argv[0], "time") == 0){
				//  Pass memory address for 2nd argument
				exitstatus = mysh_time(t);
				break;
			}
			//  Change Directory
			if(strcmp(t->argv[0], "cd") == 0){
				//  Pass memory address for 2nd argument
				mysh_cd(&t->argv[1]);
				break;
			}

			//  Fork and execute external command(s)
			exitstatus = launch_command(t);
			break;
		}
		case N_SEMICOLON:{
			print_cmdtree(t);
			execute_cmdtree(t->left);
			exitstatus = execute_cmdtree(t->right);
			break;
		} 
		default :
			fprintf(stderr,"%s: invalid NODETYPE in print_cmdtree0()\n",argv0);
			exitstatus = EXIT_FAILURE;
			break;
	}
	return exitstatus;
}

