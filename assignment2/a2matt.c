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
    // change directory to "recursepath"
    chdir(recursepath);

    printf("\nLet's do this. Going through \"%s\" now.\n", recursepath);

    // read the directory, item by item
    while ((direntry = readdir(dp)) != NULL )
    {
        // stat each thing into statbuf
        stat(direntry->d_name, &statbuf);
        // if it's a file...
        if (!(S_ISDIR(statbuf.st_mode))) {
            // do stuff with the file
            printf("The size of file \"%s\" is %d bytes\n", direntry->d_name, (int) statbuf.st_size);
        } else { // "direntry->d_name" is a directory
            // compare directory name with "." or "..", special directories
            if (strcmp(direntry->d_name, ".") == 0 || strcmp(direntry->d_name, "..") == 0) {
                // don't go into . and ..! that's the DANGER ZONE
                printf("\"%s\" is a SPECIAL directory\n", direntry->d_name);
            } else {
                printf("\"%s\" is a normal directory, descending into it\n", direntry->d_name);
                // convert "direntry->d_name" (relative directory) to absolute directory ("recursepath")
                realpath(direntry->d_name, recursepath);
                recurse_through_directory(recursepath);
            }
        }
    }
    printf("Done with that folder \"%s\"!\n\n", recursepath);
}

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

    recurse_through_directory(dirpath);
    
    exit(EXIT_SUCCESS);
}

// matt nuked code because he's just working on part 4