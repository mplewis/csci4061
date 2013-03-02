/***********************************************************************************************

 CSci 4061 Spring 2013
 Assignment# 2:   I/O Programming on UNIX/LINUX

 Student name: Christian Drews   Matthew Lewis
 Student ID:   4368668   4237332
 X500 id: drews032   lewi0486

 Operating system on which you tested your code: Linux, Unix, Solaris, MacOS
 CSELABS machine: <machine you tested on eg: xyz.cselabs.umn.edu>

 GROUP INSTRUCTION:  Please make only ONLY one  submission when working in a group.

***********************************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#define NAMESIZE 256
#define TOKENSIZE 100

int main(int argc, char *argv[])
{
    int choice = -1;
    char *input_dir_name, *dirpath, *chptr;
    struct stat statbuf;

    input_dir_name = (char *) malloc(NAMESIZE * sizeof(char));
    dirpath = (char *) malloc(NAMESIZE * sizeof(char));
    printf("SELECT THE FUNCTION YOU WANT TO EXECUTE:\n");
    printf("1. Find the 3 largest files in a directory\n");
    printf("2. List all zero length files in a directory\n");
    printf("3. Find all files with permission 777 in a directory\n");
    printf("4. Create a backup of a directory\n");
    printf("\n");
    printf("ENTER YOUR CHOICE: ");
    scanf("%d", &choice);
    printf("Enter a directory name in the current directory: ");
    scanf("%s", input_dir_name);
    
    /**********************************************************/
    /*Form a full path to the directory and check if it exists*/
    /**********************************************************/
    
    // find the absolute path of input_dir_name and put it in dirpath
    realpath(input_dir_name, dirpath);
    // debug that to stdout
    printf("Absolute path: %s\n", dirpath);

    // check if directory "dirpath" exists
    int dir_exists_error = stat(dirpath, &statbuf);
    if (dir_exists_error == -1) {
        perror("Directory does not exist, probably\n");
        exit(EXIT_FAILURE);
    } else {
        printf("Directory (or file) exists, good job\n");
        if (S_ISDIR(statbuf.st_mode)) {
            printf("Is a directory! Great job\n");
        } else {
            perror("Is not a directory. Sorry\n");
            exit(EXIT_FAILURE);
        }
    }

/* MATT IS MESSING WITH CODE HERE this is not part of the real program */

    void recurse_through_directory(char* recursepath)
    {
        // define the directory variable dp
        DIR *dp;
        if ( (dp = opendir(recursepath)) == NULL) {
            perror("Error while opening the directory\n");
            exit(EXIT_FAILURE);
        }

        // define the direntry thing
        struct dirent *direntry;
        // and totalsum
        int totalsum = 0;
        // change directory to "recursepath"
        chdir(recursepath);

        printf("\nLet's do this. Going through \"%s\" now.\n", recursepath);

        // read the directory, item by item
        while ((direntry = readdir(dp)) != NULL )
        {
            // stat each thing into statbuf
            stat(direntry->d_name, &statbuf);
            // if it's a file
            if (!(S_ISDIR(statbuf.st_mode))) {
                printf("The size of file \"%s\" is %d bytes\n", direntry->d_name, (int) statbuf.st_size);
                totalsum += (int) statbuf.st_size;
            } else { // "direntry->d_name" is a directory
                // compare directory name with "." or "..", special directories
                if (strcmp(direntry->d_name, ".") == 0 || strcmp(direntry->d_name, "..") == 0) {
                    printf("\"%s\" is a SPECIAL directory\n", direntry->d_name);
                } else {
                    printf("\"%s\" is a normal directory, descending into it\n", direntry->d_name);
                    realpath(direntry->d_name, recursepath);
                    // printf("recursepath: \"%s\"\n", recursepath);
                    recurse_through_directory(recursepath);
                }
            }
        }
        printf("Done with that folder!\n\n");
    }

    recurse_through_directory(dirpath);
    
    exit(EXIT_SUCCESS);

/* MATT IS DONE MESSING WITH CODE NOW back to the real program */

	if(choice == 1){
		printf("\nEXECUTING \"1. Find the 3 largest files in a directory\"\n");
		/********************************************************/
		/**************Function to perform choice 1**************/
		/********************************************************/
		filerecursion(dirpath, 1);
	}
	else if(choice == 2){
		printf("\nEXECUTING \"2. List all zero length files in a directory\"\n");
		/********************************************************/
		/**************Function to perform choice 2**************/
		/********************************************************/
		filerecursion(dirpath, 2);
	}

	else if(choice == 3){
		printf("\nEXECUTING \"3. Find all files with permission 777 in a directory\"\n");
		/********************************************************/
		/**************Function to perform choice 3**************/
		/********************************************************/
		filerecursion(dirpath, 3);
	}

	else if(choice == 4){
		printf("\nEXECUTING \"4. Create a backup of a directory\"\n");
		/********************************************************/
		/**************Function to perform choice 4**************/
		/********************************************************/
	}

	else{
		printf("Invalid choice\n");
		exit(1);
	}
	free(input_dir_name);
	free(dirpath);
	return 0;
}

// matt is commenting out code that is causing compilation errors
/*
int filerecursion(char *dirpath, int choice) {
  struct stat statdata;
  DIR *dpntr;
  if(choice == 1) {
  }
  else if(choice == 2) {
  }
  else if(choice == 3) {
    char accessmodes[10];
    getAccessModeString(statdata->st_uid, accessmodes);
  }
  else if(choice == 4) {
  }
  else {
    perror("Somehow you broke the program bonehead");
    exit(100);
  }
}

char *getAccessModeString ( const mode_t mode, char mstr[] ){
    sprintf(mstr, "----------");

    // Get file typeifnormation bit
    if ( S_ISLNK(mode) )   mstr[0] ='l';
    if ( S_ISDIR(mode) )   mstr[0] ='d';
    if ( S_ISCHR(mode) )   mstr[0] ='c';
    if ( S_ISBLK(mode) )   mstr[0] ='b';
 
    // Get user access bits
    if ( S_IRUSR & mode )  mstr[1] = 'r';
    if ( S_IWUSR & mode )  mstr[2] = 'w';
    if ( (S_IXUSR & mode) && !(S_ISUID & mode) )  mstr[3] = 'x';
    if ( !(S_IXUSR & mode) && (S_ISUID & mode) )  mstr[3] = 'S';
    if ( (S_IXUSR & mode) && (S_ISUID & mode) )  mstr[3] = 's';

    // Get group access bits
    if ( S_IRGRP & mode )  mstr[4] = 'r';
    if ( S_IWGRP & mode )  mstr[5] = 'w';
    if ( (S_IXGRP & mode) && !(S_ISGID & mode) )  mstr[6] = 'x';
    if ( !(S_IXGRP & mode) && (S_ISGID & mode) )  mstr[6] = 'S';
    if ( (S_IXGRP & mode) && (S_ISGID & mode) )  mstr[6] = 's';

    // Get other access bits
    if ( S_IROTH & mode )  mstr[7] = 'r';
    if ( S_IWOTH & mode )  mstr[8] = 'w';
    if ( (S_IXOTH & mode) && !(S_ISVTX & mode) )  mstr[9] = 'x';
    if ( !(S_IXOTH & mode) && (S_ISVTX & mode) )  mstr[9] = 'S';
    if ( (S_IXOTH & mode) && (S_ISVTX & mode) )  mstr[9] = 's';

}
*/