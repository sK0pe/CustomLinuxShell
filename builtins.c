#include "mysh.h"
#include <sys/time.h>
#include <dirent.h>

/*
	CITS2002 Project 2 2015
	Name:			Pradyumn Vij
	Student number:	21469477
	Date:			26/10/2015
*/

// -------------------------------------------------------------------
//	Commands run by the mysh shell program (internal commands)
// -------------------------------------------------------------------


/*
 *  mysh_exit
 *  
 *  input: CMDTREE pointer
 *
 *  returns: void
 *
 *  Exits mysh with prior exitstatus if called with no argurments.
 *  If called with arguments, exits with the numeric value
 *  of only the first argument immediately after the call to exit
 */
void mysh_exit(CMDTREE *t){
	//  More than one argument:
	if(t->argc > 1){
		exit(atoi(t->argv[1]));
	}
	//  No arguments
	else{
		exit(getPriorExitStatus());
	}
}



/*
 * 	mysh_cd
 *  
 * 	input: pointer to array of character arrays
 *
 * 	returns: integer
 *
 * 	Attempts to change directory to the first element of
 * 	input array.
 * 	Defaults to HOME directory, if input is NULL.
 * 	If no '/' found in the first element, searches CDPATH.
 *  Return describes success or failure.
 */
int mysh_cd(char **directory){
	int exitstatus;
	if(directory[0] == NULL){	//  If arg not present
		chdir(HOME);	//  Default cd to home directory
		exitstatus = EXIT_SUCCESS;
	}
	else{
		if(chdir(directory[0]) != 0){	
			// If CD fails
			// If no forward slash in directory, consider CDPATH
			if(strchr(directory[0], '/') == NULL){
				char tempCDPATH[strlen(CDPATH)+1];
				strcpy(tempCDPATH, CDPATH);
				char tryPath[MAXPATHLEN];
				char *token = strtok(tempCDPATH, ":");
				//	Loop through possible paths from CDPATH
				while(token != NULL){
					sprintf(tryPath, "%s/%s/", token, directory[0]);
					//	If directory found, exit
					if(chdir(tryPath) == 0){
						// Found path, exit successfully
						return EXIT_SUCCESS;
					}
					//	clear path buffer
					memset(tryPath, 0, sizeof tryPath);
					token = strtok(NULL,":");
				}
			}
			perror("mysh_cd");
			exitstatus = EXIT_FAILURE;
		}
		else{
			exitstatus = EXIT_SUCCESS;
		}
	}
	return exitstatus;
}

/*
 *	mysh_time
 *  
 *	input: CMDTREE pointer
 *	return: exit status of remainder of line
 *	Times all tasks that are run after the argument
 *	of time.
 *	Changes the structure of CMDTREE to reflect the
 *	removal of the first argument then reverts the changes.
 *	
 */
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

/*
 *  check_path_validity
 *
 *  input: 
 *  character pointer to colon separate string of paths
 *
 *  return:
 *  integer indicating success or failure of accessibility to
 *  all paths present in list.
 *  Helper function for mysh_set to find valid PATH and CDPATH
 *  strings.
 */
int check_path_validity(char *pathList){
	char tempList[strlen(pathList)+1];
	strcpy(tempList, pathList);
	char *path = strtok(tempList, ":");
	while(path != NULL){
		if(opendir(path) == NULL){
			perror("opendir");
			fprintf(stderr, "%s could not be accessed", path);
			return EXIT_FAILURE;
		}
		path = strtok(NULL, ":");
	}
	return EXIT_SUCCESS;
}

/*
 *  mysh_set
 *
 *  input:
 *  integer described the number arguments minus set
 *  Pointer to an array of character pointers
 *  return:
 *  integer determining success of task
 *
 *  Funciton allows change of internal variables HOME, PATH and CDPATH.
 *  Paths should be colon separated while the HOME variable can only have
 *  a single path.  Changes do not carry over to next launch of mysh.
 *
 */
int mysh_set(int argc, char **argv){
	int variableChangeStatus;
	if(argc != 2){
		fprintf(stderr, "set: requires 2 arguments\n"
			"An internal variable to change: (HOME, PATH or CDPATH)\n"
			"and the replacement directory path for HOME or ':' "
			"separated\npaths for PATH variables.\n");
		return EXIT_FAILURE;
	}
	if(strcmp(argv[0], "HOME") == 0){
		//  Check if valid directory path
		if(opendir(argv[1]) == NULL){
			perror("opendir");
			return EXIT_FAILURE;
		}
		HOME = argv[1];
		variableChangeStatus = EXIT_SUCCESS;
	}
	if(strcmp(argv[0], "PATH") == 0){
		//  Check if paths are valid
		if(check_path_validity(argv[1]) == 0){
			PATH = argv[1];
			variableChangeStatus = EXIT_SUCCESS;
		}
		else {
			variableChangeStatus = EXIT_FAILURE;
		}
	}
	if(strcmp(argv[0], "CDPATH") == 0){
		//  Check if paths are valid
		if(check_path_validity(argv[1]) == 0){
			CDPATH = argv[1];
			variableChangeStatus = EXIT_SUCCESS;
		}
		else{
			variableChangeStatus = EXIT_FAILURE;
		}
	}
	return variableChangeStatus;
}
