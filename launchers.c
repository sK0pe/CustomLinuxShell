#include "mysh.h"
#include <sys/types.h>
/*
	CITS2002 Project 2 2015
	Name(s):		Pradyumn Vij
	Student number(s):	21469477
	Date:		date-of-submission
*/

/*
 *  launch_command
 *  
 *  input: CMDTREE pointer
 *  return: exit status of an attempt at forking and executing
 *  a program.
 *  Funciton works as a single foreground launcher.
 *  Forks the parent then runs exec on the child and loops and
 *  waits for child specific exit, wait is broken by normal exit
 *  or if the child has been signalled to close.
 */
int launch_command(CMDTREE *t){
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

/*
 *  launch_background
 *  
 *  input: CMDTREE pointer
 *  return: integer indicating whether fork successful or failed
 *  Variant of function "launch_command", however does not inlcude
 *  the parent waiting, only a child being forked and replaced
 *  with a program, allowing it to run in the background as a child
 *  of shell, returns to a wait in function "execute_cmdtree"
*/
int launch_background(CMDTREE *t){
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
	//  Background process only fails on forking
	return EXIT_SUCCESS;
}

/*
 *launch_subshell 
 *
 *input: CMDTREE Pointer
 *return: Exit status of the function run in the subshell
 *Creates a child shell from parent shell and runs commands
 *within the child shell.
 *Returns the exit status of the child shell commands.
 */
int launch_subshell(CMDTREE *t){
	int shellStatus;
	int childStatus;
	pid_t waitID;
	// Fork program to try to make a child process
	pid_t programID = fork();
	if(programID < 0){	// parent checks if fork() failed
		//  Fork has failed
		perror("fork");
		return EXIT_FAILURE;
	}

	if(programID == 0){
		//  child shell
		shellStatus = execute_cmdtree(t);
	}
	else{
		//  parent shell waiting for child exit
		do{
			//  Make parent wait for child shell to end
			waitID = waitpid(programID, &childStatus, WUNTRACED);
			//  Check if wait exited with error
			if(waitID == -1){
				perror("waitpid");
				exit(EXIT_FAILURE);
			}
		}
		//  Wait until child exits or is signalled to exit
		while(!WIFEXITED(childStatus) && !WIFSIGNALED(childStatus));
	}
	//  Background process only fails on forking
	return shellStatus;
}
