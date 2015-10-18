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
{
	int exitstatus;	
	int status;  //  used by wait, to check on child process
	pid_t programID;

	programID = fork();

	if(programID < 0){
		//fprintf(stderr, "Forking %s failed\n", *t->argv);
		perror("fork");
		exit(EXIT_FAILURE);
	}
	//  If fork succeeds
	if(programID == 0){	//  child process successfully initiated
		if(execvp(*t->argv, t->argv) < 0){	//  if execute fails
			//fprintf(stderr, "Execution of %s failed.\n", *t->argv);
			perror("mysh");
			exit(EXIT_FAILURE);
		}
	}
	else{	// parent process (mysh) waiting for child to end
		while(wait(&status) != programID);
	}
	exitstatus = 1;
	return exitstatus;
}

//Execute CMD tree by traversing the graph. 
int execute_cmdtree(CMDTREE *t) {
	int exitstatus;
	CMDTREE* temp = NULL;

	//Keep searching the tree for a command.  Exit loop by returning exitstatus.
	while(1) {	
		switch (t->type) {
			case N_COMMAND :
				exitstatus = execute_command(t);  //Iff only one command, execute command then return exitstatus.
				return exitstatus;
				break;
			
			case N_AND :
				//Check tree node has valid left and right children
				if(!t->left || !t->right) {
					fprintf(stderr,"%s: invalid NODETYPE in print_cmdtree0()\n",argv0);
					exit(1);
					break;	
				}	
				//Execute left childs command, then record its exit status.  If no command, error in tree.
				if(t->left->type == N_COMMAND) 
					exitstatus = execute_command(t->left);


				//If exitstatus of left child command is false, then right child is not executed.  If tree position
				//to the left of N_SEMICOLON node go to right of N_SEMICOLON node, position stored by temp.  Otherwise
				//tree traversal is finished, return with exitstatus.
				if(!exitstatus) {
					if(temp) { 
						t=temp->right;
						temp=NULL;
					}
					else return exitstatus;
					continue;
				}
			
				//If right child is a command, execute the command.  Then if left of ';' point to node right of ';'. 
				//Otherwise, tree traversal is finished, return exitstatus.	
				if(t->right->type == N_COMMAND ) {
					exitstatus = execute_command(t->right);
					if(temp) {
						t=temp->right;
						temp=NULL;
					}
					else return exitstatus;
					continue;
				}
				
					
				else {	
					if (t->right) t=t->right;
					else return exitstatus;
					continue;
				}
				break;
			
			
			case N_OR :
				//Check left right child nodes not NULL
				if(!t->left || !t->right) {
					fprintf(stderr,"%s: invalid NODETYPE in print_cmdtree0()\n",argv0);
					exit(1);
					break;	
				}	
				//Execute left node.  If left child node is not a command there is an error.
				if(t->left->type == N_COMMAND) 
					exitstatus = execute_command(t->left);
				//If command is successful, do not execute right child node. If left of N_SEMMICOLON node
				//move to the right.  Otherwise, last command on tree. Return exitvalue and exit tree.
				if(exitstatus) {
					if(temp) { 
						t=temp->right;
						temp=NULL;
					}
					else return exitstatus;
					continue;
				}
				//Execute right child nodes command if it is a command.
				if(t->right->type == N_COMMAND ) {
					exitstatus = execute_command(t->right);
					if(temp) {
						t=temp->right;
						temp=NULL;
					}
					else return exitstatus;
					continue;
				}	
				//If right child node is not a command, move to that node.
				else {	
					if (t->right) t=t->right;
					continue;
				}			
				break;
			

			case N_SEMICOLON :
				//Check if left or right child nodes are NULL
				if(!t->left || !t->right) {
					fprintf(stderr,"%s: invalid NODETYPE in print_cmdtree0()\n",argv0);
					exit(1);
					break;
				} 
				//If left node is a command, execute command.
				if(t->left->type == N_COMMAND) {
					exitstatus = execute_command(t->left);	
				}
				//Otherwise, travel to that node.  Remember pointer to N_SEMICOLON node.  After executing
				//left tree, temp can be used to remember right side of tree for execution.
				else {
					temp=t;
					t=t->left;
					continue;
				}
				//If right child node is a command, execute it.  Then return exitstatus, tree is finished.
				if(t->right->type == N_COMMAND) {
					exitstatus = execute_command(t->right);
					return exitstatus;
				}
				//Otherwise, travel to right child node.
				else {
					temp=NULL;
					t=t->right;
					continue;
				}	
				break;
			
			default :
				fprintf(stderr,"%s: invalid NODETYPE in print_cmdtree0()\n",argv0);
				exit(1);
				break;
	  	}
	}
	return exitstatus;
}
