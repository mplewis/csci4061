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
        printf("Directory does not exist, probably\n");
        exit(EXIT_FAILURE);
    } else {
        printf("Directory (or file) exists, good job\n");
        if (S_ISDIR(statbuf.st_mode)) {
            printf("Is a directory! Great job\n");
        } else {
            printf("Is not a directory. Sorry\n");
            exit(EXIT_FAILURE);
        }
    }

/* MATT IS MESSING WITH CODE HERE this is not part of the real program 

    // let's figure out this recursion code yo

    // change directory to "dirname"
    chdir(dirname);
    // read it and stuff? i dunno
    while( (direntry = readdir(dp)) != NULL )
    {
        // stat each thing into statbuf
        stat(direntry->d_name, &statbuf);
        // if it's a file... do shit
        if(!(S_ISDIR(statbuf.st_mode)))
        {
            printf("The size of file %s is :%d bytes\n",direntry->d_name,(int) statbuf.st_size);
            totalsum += (int) statbuf.st_size;
        }
    }

    exit(EXIT_SUCCESS);

MATT IS DONE MESSING WITH CODE NOW back to the real program */

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

int filerecursion(char *dirpath, int choice) {
  struct stat statdata;
  struct dirend *dentry;
  DIR *dpntr;
  dpntr = opendir(dirpath);
  dentry = readdir(dpntr);
  if(choice == 1) {
  }
  else if(choice == 2) {
  }
  else if(choice == 3) {
   	 char accessmodes[10];
   	 char filepathname[256];
   	 while(dentry != 0) {
		sprintf(filepathname, "%s", dirpath);
   		if(!(stat(filepathname, &statdata))) {		
    			getAccessModeString(statdata->st_uid, accessmodes);
			if(accessmodes == "111111111") {
				printf("File %s has permission 777", filepathname);
			}
    		}
    		else {
			fprintf(stderr, "Getting stat for %s", filepath);
		}
	}
  }
  else if(choice == 4) {
  }
  else {
    perror("Somehow you broke the program bonehead");
    exit(100);
  }
}

char *getAccessModeString ( const mode_t mode, char mstr[] ){
    sprintf(mstr, "---------");
 
    /* Get user access bits         */
    if ( S_IRUSR & mode )  mstr[0] = '1';
    if ( S_IWUSR & mode )  mstr[1] = '1';
    if ( S_IXUSR & mode )  mstr[2] = '1';

    /* Get group access bits         */
    if ( S_IRGRP & mode )  mstr[3] = '1';
    if ( S_IWGRP & mode )  mstr[4] = '1';
    if ( S_IXGRP & mode )  mstr[5] = '1';

    /* Get other access bits         */
    if ( S_IROTH & mode )  mstr[6] = '1';
    if ( S_IWOTH & mode )  mstr[7] = '1';
    if ( S_IXOTH & mode )  mstr[8] = '1';

}

