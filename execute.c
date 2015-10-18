#include "mysh.h"
#include <sys/types.h>
#include <sys/wait.h>

/*
   CITS2002 Project 2 2015
   Name(s):		student-name1 (, student-name2)
   Student number(s):	student-number-1 (, student-number-2)
   Date:		date-of-submission
 */

// -------------------------------------------------------------------

//  THIS FUNCTION SHOULD TRAVERSE THE COMMAND-TREE and EXECUTE THE COMMANDS
//  THAT IT HOLDS, RETURNING THE APPROPRIATE EXIT-STATUS.
//  READ print_cmdtree0() IN globals.c TO SEE HOW TO TRAVERSE THE COMMAND-TREE

//Bundle code for executing command into a single method.  This way code is reused by execute_cmdtree0().
int execute_command(CMDTREE *t)
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
