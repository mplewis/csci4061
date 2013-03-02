/***********************************************************************************************

 CSci 4061 Spring 2013
 Assignment# 2:   I/O Programming on UNIX/LINUX

 Student name: <full name of first student>   <full name of second student>
 Student ID:   <first student's ID>   (second student's ID>
 X500 id: <id1>, <id2 (optional)>

 Operating system on which you tested your code: Linux, Unix, Solaris, MacOS
 CSELABS machine: <machine you tested on eg: xyz.cselabs.umn.edu>

 GROUP INSTRUCTION:  Please make only ONLY one submission when working in a group.

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


    if(choice == 1){
        printf("\nEXECUTING \"1. Find the 3 largest files in a directory\"\n");
        /********************************************************/
        /**************Function to perform choice 1**************/
        /********************************************************/

    }

    else if(choice == 2){
        printf("\nEXECUTING \"2. List all zero length files in a directory\"\n");
        /********************************************************/
        /**************Function to perform choice 2**************/
        /********************************************************/

    }

    else if(choice == 3){
        printf("\nEXECUTING \"3. Find all files with permission 777 in a directory\"\n");
        /********************************************************/
        /**************Function to perform choice 3**************/
        /********************************************************/

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
