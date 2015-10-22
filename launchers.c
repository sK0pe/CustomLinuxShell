#include "mysh.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/*
	CITS2002 Project 2 2015
	Name(s):		Pradyumn Vij
	Student number(s):	21469477
	Date:		date-of-submission
*/

// -------------------------------------------------------------------
//	Functions used by execute.c to execute programs in the CMDTREE
// -------------------------------------------------------------------


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
		char *possiblePath = strtok(tempPATH, ":");
		//	Loop through possible paths from PATH
		while(possiblePath != NULL){
			sprintf(tryPath,"%s/%s", possiblePath, command);
			//	Attempt launch
			//  If successful this program will no longer exist
			execv(tryPath, argv);
			//	clear path buffer
			memset(tryPath, 0, sizeof tryPath);
			//  Assign next possible buffer
			possiblePath = strtok(NULL,":");
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
 *  initialise_file_descriptors
 *
 *  input: CMDTREEE pointer
 *  return: void 
 *  
 *  Helper function for launch_command and launch_background.
 *  Checks if command tree requires files to be read or written to, 
 *  if so initiates file descriptors and redirects to STDIN and
 *  STDOUT
 */
void initialise_file_descriptors(CMDTREE *t){
	//  Integers used by open()
	int inDescriptor;
	int outDescriptor;
	//  Check if user wants to read input from file
	if(t->infile != NULL){
		//  Read input from file
		//  Open into read only descriptor
		inDescriptor = open(t->infile, O_RDONLY);
		if(inDescriptor < 0){
			perror("Open Input file");
			fprintf(stderr, "Program %s could not read file: %s\n", 
				t->argv[0], t->infile);
			exit(EXIT_FAILURE);
		}
		//  Replace STDIN with infile
		if(dup2(inDescriptor,0) < 0){
			perror("dup2 infile");
			exit(EXIT_FAILURE);
		}
		//  Close file descriptor
		if(close(inDescriptor) < 0){
			perror("close infile");
			exit(EXIT_FAILURE);
		}
	}
	
	//  Check if user wants output written to file
	if(t->outfile != NULL){
		//  Open into file descriptor, create if doesn't exist
		//  overwrite if does exist
		//  Check if user wants to append
		outDescriptor = t->append ? 
		open(t->outfile, O_WRONLY | O_CREAT | O_APPEND) : 
		open(t->outfile, O_WRONLY | O_CREAT | O_TRUNC);

		if(outDescriptor == -1){
			perror("Open Input file");
			fprintf(stderr, "Program %s could not read file: %s\n", 
				t->argv[0], t->infile);
			exit(EXIT_FAILURE);
		}
		//  Replace STDOUT with outfile
		if(dup2(outDescriptor,1) < 0){
			perror("dup2 outfile");
			exit(EXIT_FAILURE);
		}
		//  Close file descriptor
		if(close(inDescriptor) < 0){
			perror("close outfile");
			exit(EXIT_FAILURE);
		}
	}
}

/*
 *  launch_command
 *  
 *  input: CMDTREE pointer
 *  return: exit status of an attempt at forking and executing
 *  a program.
 *
 *  Function works as a single foreground launcher.
 *  Forks parent, runs exec on child or initiates subshell and
 *  and launches remaining command tree.
 *  Waits for child, specific to child exit.
 */
int launch_command(CMDTREE *t){
	int launchStatus;  // return status for the function
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
		//  Child process scope
		//  Handle file descriptors if required
		initialise_file_descriptors(t);

		if(t->type == N_SUBSHELL){
			//  Fork a child shell and execute remaining
			//  CMDTREE in child shell, launch status represents
			//  child shell's exit status 
			launchStatus = execute_cmdtree(t->left);
		}
		else{
			//  CMDTREE type is N_COMMAND otherwise
			//  Replace child with program or exit
			execute_command(t->argv[0], t->argv);
		}
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
		//  Handle file descriptors if required
		initialise_file_descriptors(t);
		//  Replace child with program or exit 
		execute_command(t->argv[0], t->argv);
	}
	//  Background process only fails on forking
	return EXIT_SUCCESS;
}
