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
#include <sys/time.h>
#include <time.h>
#include <dirent.h>
#include <errno.h>
#define NAMESIZE 256
#define BUFSIZE 256
#define PATHSIZE 1024
#define TOKENSIZE 100
#define BACKUP_SUFFIX ".bak"

struct stat statbuf;

// get current date and time in the format 'Mar-03-2013-20-24-04' and place it
// in 'buffer' with a max length of 'bufsize'
void time_to_buf(char* buffer, int bufsize)
{
    char* datetime;
    int retval;
    time_t clocktime;
    struct tm *timeinfo;
    time (&clocktime);
    timeinfo = localtime( &clocktime );
    strftime(buffer, bufsize, "%b-%d-%Y-%H-%M-%S", timeinfo); 
}

// change directory to 'changeto', store current directory in 'newpath'
// (with max len 'bufsize')
int change_dir(char* changeto, char* newpath, int bufsize) {
    if ((chdir(changeto)) == 0) {
        if (getcwd(newpath, bufsize)) {
            return 0;
        } else {
            return -2;
        }
    }
    else  { 
        return -1;
    }  
}

// create a symbolic link at 'linksrc' pointing to 'linkdest'.
// mind the argument order!
int create_symlink(char* linkdest, char* linksrc)
{
    int status;
    status = symlink(linkdest, linksrc);
    return status;
}

int get_symlink_dest(char* symlinkpath, char* linkinfo, int bufsize)
{
    int retval = 0;
    // readlink will obtain and store the link information in linkinfo array
    // returns the number of bytes to retval, or -1 if error
    retval = readlink(symlinkpath, linkinfo, bufsize);
    printf("%i\n", retval);
    if (retval == -1) {
        // symlink info error
        return -1;
    } else {
        // append the null that readlink doesn't
        linkinfo[retval] = '\0';
        return 0;
    }
}

void recurse_through_directory_backup(char* recursepath)
{
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
    change_dir("..", recursepath, PATHSIZE);
    printf("Current directory: \"%s\"\n", recursepath);
}

int make_backup_directory(char *backupsrc) {
    char* backupdest;
    backupdest = (char*) malloc(PATHSIZE * sizeof(char));
    // copy backupsrc to backupdest and append the backup suffix
    strcat(backupdest, backupsrc);
    strcat(backupdest, BACKUP_SUFFIX);

    printf("Backing up %s to %s\n", backupsrc, backupdest);

    // FIXME if time: permissions for new directory
    // create new directory.bak, catching errors
    int errcreate = mkdir(backupdest, 0755);
    if (errcreate != 0) {
        // something went wrong
        if (errno == EEXIST) {
            // directory.bak already exists!
            printf("%s already exists!\n", backupdest);
        }
    }

}

int main(int argc, char *argv[])
{
    /*
    char buffer[BUFSIZE];
    time_to_buf(buffer, BUFSIZE);
    printf("Current date and time is: %s\n", buffer);

    char currpath[PATHSIZE];
    getcwd(currpath, PATHSIZE);
    printf("%s\n", currpath);
    change_dir("..", currpath, PATHSIZE);
    printf("%s\n", currpath);
    
    int err = mkdir("testdir2", 0775);
    if (err) {
        printf("%i: %s\n", err, strerror(errno));
    }

    create_symlink("test.sh", "sym_link_test.sh");

    char *linkout;
    linkout = (char *) malloc(PATHSIZE * sizeof(char));
    get_symlink_dest("LINK_TO_MATT.sh", linkout, PATHSIZE);
    printf("%s\n", linkout);

    rename("testdir", "testdir_bak");
    */

    int choice = -1;
    char *input_dir_name, *dirpath, *chptr;

    input_dir_name = (char *) malloc(NAMESIZE * sizeof(char));
    dirpath = (char *) malloc(PATHSIZE * sizeof(char));
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

    make_backup_directory(dirpath);

    exit(EXIT_SUCCESS);
}

// matt nuked code because he's just working on part 4