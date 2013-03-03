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

struct stat statbuf;

void recurse_through_directory_backup(char* recursepath)
{
    char *newpath;
    newpath = (char *) malloc(NAMESIZE * sizeof(char));

    // define the directory variable dp
    DIR *dp;
    if ((dp = opendir(recursepath)) == NULL) {
        perror("Error while opening the directory\n");
        exit(EXIT_FAILURE);
    }

    // define the dirent for the ORIGINAL directory
    struct dirent *origdent;
    // define the dirent for the BACKUP directory
    struct dirent *backupdent;
    // change directory to "recursepath"
    chdir(recursepath);

    printf("Going through \"%s\" now\n", recursepath);

    // read the directory, item by item
    while ((origdent = readdir(dp)) != NULL )
    {
        // stat each thing into statbuf while checking for errors
        if (lstat(origdent->d_name, &statbuf) == -1) {
            perror("Error while opening the file\n");
            exit(EXIT_FAILURE);
        }
        // if it's a file...
        if (S_ISREG(statbuf.st_mode)) {
            // do stuff with the file
            printf("\tBacking up \"%s\"\n", origdent->d_name);
        } else if (S_ISLNK(statbuf.st_mode)) {
            // do stuff with the symlink
            printf("\tBacking up \"%s\" (SYMLINK)\n", origdent->d_name);
        } else { // "origdent->d_name" is a directory
            // compare directory name with "." or "..", special directories
            if (strcmp(origdent->d_name, ".") == 0 || strcmp(origdent->d_name, "..") == 0) {
                // don't go into . and ..! that's the DANGER ZONE
                // printf("\"%s\" is a SPECIAL directory\n", origdent->d_name);
            } else {
                printf("\tCreating \"%s\" and descending...\n", origdent->d_name);
                // convert "origdent->d_name" (relative directory) to absolute directory ("recursepath")
                realpath(origdent->d_name, recursepath);
                recurse_through_directory_backup(recursepath);
            }
        }
    }
    printf("Done with that directory \"%s\"! Going up\n", recursepath);

    // GO UP A DIRECTORY
    // copy recursepath to newpath
    strcpy(newpath, recursepath);
    strcat(recursepath, "/..");
    realpath(recursepath, newpath);
    chdir(newpath);
    printf("Now in directory \"%s\"\n", newpath);
    
}

int main(int argc, char *argv[])
{
    int choice = -1;
    char *input_dir_name, *dirpath, *chptr;

    input_dir_name = (char *) malloc(NAMESIZE * sizeof(char));
    dirpath = (char *) malloc(NAMESIZE * sizeof(char));
    printf("SELECT THE FUNCTION YOU WANT TO EXECUTE:\n");
    printf("1. Find the 3 largest files in a directory\n");
    printf("2. List all zero length files in a directory\n");
    printf("3. Find all files with permission 777 in a directory\n");
    printf("4. Create a backup of a directory\n");
    printf("\n");
    // printf("ENTER YOUR CHOICE: ");
    // scanf("%d", &choice);
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

    recurse_through_directory_backup(dirpath);
    
    exit(EXIT_SUCCESS);
}

// matt nuked code because he's just working on part 4