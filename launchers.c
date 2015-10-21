#include "mysh.h"
#include <sys/types.h>
/*
	CITS2002 Project 2 2015
	Name(s):		Pradyumn Vij
	Student number(s):	21469477
	Date:		date-of-submission
*/

/*
 *  execute_command 
 *  
 *  input: character array pointer and array of character array pointer
 *  return: void
 *  
 *  Helper function for child scope launchers functions, 
 *  checks PATH if user enters a command not containing a '/'
 *  Also calls perror and exits child if fails
 */
void execute_command(char *command, char **argv){
	//  If command doesn't include a '/' consider PATH directories
	if(strchr(command, '/') == NULL){
		// Make copy of PATH as tokenizer is destructive
		char tempPATH[strlen(PATH)+1];
		strcpy(tempPATH, PATH);
		char tryPath[MAXPATHLEN];
		char *token = strtok(tempPATH, ":");
		//	Loop through possible paths from PATH
		while(token != NULL){
			strcpy(tryPath, token);
			strcat(tryPath, "/");
			strcat(tryPath, command);
			//	Attempt launch
			execv(tryPath, argv);
			//	clear path buffer
			memset(tryPath, 0, sizeof tryPath);
			//  Assign next possible buffer
			token = strtok(NULL,":");
		}
	}
	else{
		// command includes a '/'
		execv(command, argv);
	}
	perror(command);
	exit(EXIT_FAILURE);
}



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
		//  Try and replace child with program
		//  If file execution fails exit child 
		execute_command(t->argv[0], t->argv);
	}
	else{	
		//  parent process scope 
		//  Make parent wait for specific child to end
		waitID = waitpid(programID, &childStatus, WUNTRACED);
		//  Check if wait exited with error
		if(waitID == -1){
			perror("waitpid");
			exit(EXIT_FAILURE);
		}
		//  Interpret child exit as success or failure
		if(WIFEXITED(childStatus)){
			//  Assign child exit stat to output
			launchStatus = WEXITSTATUS(childStatus);
		}
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
		//  If file execution fails exit child
		execute_command(t->argv[0], t->argv);
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
		//  Make parent wait for child shell to end
		waitID = waitpid(programID, &childStatus, WUNTRACED);
		//  Check if wait exited with error
		if(waitID == -1){
			perror("waitpid");
			exit(EXIT_FAILURE);
		}
		//  Interpret child shell exit as success or failure
		if(WIFEXITED(childStatus)){
			//  Assign child exit stat to output
			shellStatus = WEXITSTATUS(childStatus);
		}
	}
	//  Background process only fails on forking
	return shellStatus;
}
