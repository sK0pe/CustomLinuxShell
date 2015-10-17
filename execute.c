
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

int execute_command(CMDTREE *t)
{
	int exitstatus;
	switch(t->type){
		case N_COMMAND:{
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
			break;
		}

		default :
			fprintf(stderr,"%s: invalid NODETYPE in print_cmdtree0()\n",argv0);
			exit(1);
			break;
	}
	
	/*
	int  exitstatus;
	if(t == NULL) {			// hmmmm, a that's problem
		exitstatus	= EXIT_FAILURE;
  }
	else{				// normal, exit commands
		exitstatus	= EXIT_SUCCESS;
  }
	*/
  return exitstatus;
}

int execute_cmdtree(CMDTREE *t) {
	int exitstatus;
	CMDTREE* temp = NULL;
	while(1) {	
		switch (t->type) {
			case N_COMMAND :
				exitstatus = execute_command(t);
				return exitstatus;
				break;
			
			case N_AND :
				if(!t->left || !t->right) {
					fprintf(stderr,"%s: invalid NODETYPE in print_cmdtree0()\n",argv0);
					exit(1);
					break;	
				}	
				if(t->left->type == N_COMMAND) 
					exitstatus = execute_command(t->left);
			
				if(!exitstatus) {
					if(temp) t=temp->right;
					else return exitstatus;
					continue;
				}
				
				if(t->right->type == N_COMMAND ) {
					exitstatus = execute_command(t->right);
					if(temp) t=temp->right;
					else return exitstatus;
					continue;
				}
				else {
					temp=NULL;
					if (t->right) t=t->right;
					continue;
				}
				break;
			
			
			case N_OR :
				if(!t->left || !t->right) {
					fprintf(stderr,"%s: invalid NODETYPE in print_cmdtree0()\n",argv0);
					exit(1);
					break;	
				}	
				if(t->left->type == N_COMMAND) 
					exitstatus = execute_command(t->left);
				if(exitstatus) {
					if(temp) t=temp->right;
					else return exitstatus;
					continue;
				}
				if(t->right->type == N_COMMAND ) {
					exitstatus = execute_command(t->right);
					if(temp) t=temp->right;
					else return exitstatus;
					continue;
				}
				else {
					temp=NULL;
					if (t->right) t=t->right;
					continue;
				}			
				break;
			

			case N_SEMICOLON :
				if(!t->left || !t->right) {
					fprintf(stderr,"%s: invalid NODETYPE in print_cmdtree0()\n",argv0);
					exit(1);
					break;
				} 
				if(t->left->type == N_COMMAND) {
					exitstatus = execute_command(t->left);	
				}
				else {
					temp=t;
					t=t->left;
					continue;
				}
				if(t->right->type == N_COMMAND) {
					exitstatus = execute_command(t->right);
					return exitstatus;
				}
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
