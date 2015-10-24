#include "mysh.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

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
static void execute_command(char *command, char **argv){
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
 *  safe_close
 *  
 *  input: character array pointer
 *  return: void
 *  
 *  Helper function for calling close, closes file descriptors
 *  with error checking.
 */
 void safe_close(char *program_message, int *file_descriptor){
 	if(close(*file_descriptor) < 0){
 		perror(program_message);
 		exit(EXIT_FAILURE);
 	}
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
static void initialise_file_descriptors(CMDTREE *t){
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
		safe_close("close infile", &inDescriptor);
	}
	
	//  Check if user wants output written to file
	if(t->outfile != NULL){
		//  Open into file descriptor, create if doesn't exist
		//  overwrite if does exist
		//  Check if user wants to append
		outDescriptor = t->append ? 
		open(t->outfile, O_WRONLY | O_CREAT | O_APPEND) : 
		open(t->outfile, O_WRONLY | O_CREAT | O_TRUNC);

		if(outDescriptor < 0){
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
		safe_close("close outfile", &outDescriptor);
	}
}

/*
 *  command_or_subshell
 *  input: CMDTREE pointer
 *  return: void 
 *  
 *  Helper function that executes a pointer appropriate to
 *  N_SUBSHELL or N_COMMAND and also initialises I/O file
 *  handles if required.  Expects to be launched within a
 *  child scope.
 */
 static void command_or_subshell(CMDTREE *t){
 	//  Handle file descriptors if required
	initialise_file_descriptors(t);
	switch(t->type){
		case N_SUBSHELL:{
			//  Fork a child shell and execute remaining CMDTREE in child 
			//   shell, exit with left branch exit status.
			exit(execute_cmdtree(t->left));
		}
		case N_COMMAND:{
			//  Replace child with program or exit
			execute_command(t->argv[0], t->argv);
		}
		case N_PIPE:{
			//  Launch pipe again till command or subshell found
			launch_pipe(t);
		}
		default:{
			fprintf(stderr,"%s: invalid NODETYPE in launchers\n",argv0);
			exit(EXIT_FAILURE);
			break;
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
		//initialise_file_descriptors(t);
		//  Execute subshell or command
		command_or_subshell(t);
		//if(t->type == N_COMMAND){
			//execute_command(t->argv[0], t->argv);
		//}
		/*else{
			launchStatus = execute_cmdtree(t->left);
			printf("launchStatus = %d\n", launchStatus);
			exit(launchStatus);
		}*/
	}
	else{
		//  Parent process scope 
		//  Make parent wait for specific child to end
		if(waitpid(programID, &childStatus, WUNTRACED) < 0){
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


void launch_subshell(CMDTREE *t, int *exitStatus){
	int childStatus;
	pid_t programID = fork();
	if(programID < 0){
		perror("fork");
		exit(EXIT_FAILURE);
	}
	if(programID == 0){
		initialise_file_descriptors(t);
		exit(execute_cmdtree(t->left));
	}
	else{
		if(waitpid(programID, &childStatus,0) < 0){
			perror("waitpid");
			exit(EXIT_FAILURE);
		}
		//  Interpret child exit as success or failure
		if(WIFEXITED(childStatus)){
			//  Assign child exit stat to output
			*exitStatus = WEXITSTATUS(childStatus);
		}
	}
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
		//  Execute subshell or command
		command_or_subshell(t);
	}
	//  Background process only fails on forking
	return EXIT_SUCCESS;
}


int launch_pipe(CMDTREE *t){
	int pipeResult;
	//int shellStatus1, shellStatus2;
	int pipeStatus1, pipeStatus2;
	pid_t program1, program2;
	int pipeFD[2];  // required array for pipe
	//  Create pipe
	if(pipe(pipeFD) < 0){
		perror("mysh: pipe");
		return EXIT_FAILURE;
	}
	//  Fork Parent
	if((program1 = fork()) == 0){
		//  Child Scope (also has own pipe)
		//  Replace child's STDOUT with pipe input
		if(dup2(pipeFD[1], 1) < 0){
			perror("dup2: Could not replace STDOUT with pipe in.");
			exit(EXIT_FAILURE);
		}
		// Close pipe
		safe_close("close: Could not replace STDOUT with pipe in.", &pipeFD[1]);
		safe_close("close: Could not replace STDOUT with pipe in.", &pipeFD[0]);
		//  Execute subshell or command on left branch
		command_or_subshell(t->left);
	}
	//  Fork Parent again
	else if((program2 = fork()) == 0){
		//  Second Child's scope (also has own pipe)
		//  Replace child's STDIN with pipe output
		if(dup2(pipeFD[0], 0) < 0){
			perror("dup2: Could not replace STDIN with pipe out.");
			exit(EXIT_FAILURE);
		}
		// Close pipe
		safe_close("close: Could not replace STDIN with pipe out.", &pipeFD[1]);
		safe_close("close: Could not replace STDIN with pipe out.", &pipeFD[0]);
		//  Execute subshell or command on right branch
		command_or_subshell(t->right);
	}
	else{
		//  Parent Scope
		//  Close parent's pipe
		safe_close("close: Pipe in not safely closed.", &pipeFD[1]);
		safe_close("close: Pipe out not safely closed.", &pipeFD[0]);
		//  Wait for children to return
		if(waitpid(program1, &pipeStatus1, 0) < 0 || waitpid(program2, &pipeStatus2, 0) < 0){
			perror("pipe waitpid");
			exit(EXIT_FAILURE);
		}
		//  If either exitstatus is not successful
		if(WIFEXITED(pipeStatus1) && WIFEXITED(pipeStatus2)){
			if(!WEXITSTATUS(pipeStatus1) && !WEXITSTATUS(pipeStatus2)){
				pipeResult = EXIT_SUCCESS;
			}
			else pipeResult = EXIT_FAILURE;
		}
	}
	return pipeResult;
}