#include "mysh.h"

/*
   CITS2002 Project 2 2015
   Name(s):             student-name1 (, student-name2)
   Student number(s):   student-number-1 (, student-number-2)
   Date:                date-of-submission
 */


//  THREE INTERNAL VARIABLES (SEE mysh.h FOR EXPLANATION)
char	*HOME, *PATH, *CDPATH;

char	*argv0		= NULL;		// the program's name
bool	interactive	= false;

// ------------------------------------------------------------------------

void check_allocation0(void *p, char *file, const char *func, int line){
	if(p == NULL) {
		fprintf(stderr, "%s, %s() line %i: unable to allocate memory\n",file, func, line);exit(2);
	}
}

//  REPORT ANY INPUT/OUTPUT REDIRECTION ASSOCIATED WITH A COMMAND NODE
static void print_redirection(CMDTREE *t){
	if(t->infile != NULL) printf("< %s ",t->infile);
	if(t->outfile != NULL) {
		if(t->append == false){
			printf(">");
		}
		else{
			printf(">>");
			printf(" %s ",t->outfile);
		}
	}
}

//  PRINT THE COMMANDS AND CONTROL-FLOW SEQUENCES IN THE INDICATED COMMAND NODE
void print_cmdtree0(CMDTREE *t)
{
	if(t == NULL){
		printf("<nullcmd> ");
		return;
	}

	switch (t->type) {
		case N_COMMAND :
			for(int a=0 ; a < t->argc ; a++){
				printf("%s ", t->argv[a]);
				print_redirection(t);
			}
			break;

		case N_SUBSHELL :
			printf("( "); 
			print_cmdtree0(t->left); 
			printf(") ");
			print_redirection(t);
			break;

		case N_AND :
			print_cmdtree0(t->left);
			printf("&& ");
			print_cmdtree0(t->right);
			break;

		case N_OR :
			print_cmdtree0(t->left);
			printf("|| ");
			print_cmdtree0(t->right);
			break;

		case N_PIPE :
			print_cmdtree0(t->left);
			printf("| ");
			print_cmdtree0(t->right);
			break;
		
		case N_SEMICOLON :
			print_cmdtree0(t->left);
			printf("; ");
			print_cmdtree0(t->right);
			break;
		
		case N_BACKGROUND :
			print_cmdtree0(t->left);
			printf("& ");
			print_cmdtree0(t->right);
			break;
		
		default :
			fprintf(stderr,"%s: invalid NODETYPE in print_cmdtree0()\n",argv0);
			exit(1);
			break;
  }
}
