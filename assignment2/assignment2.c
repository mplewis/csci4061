/***********************************************************************************************

 CSci 4061 Spring 2013
 Assignment# 2:   I/O Programming on UNIX/LINUX

 Student name: Christian Drews   Matthew Lewis
 Student ID:   4368668   4237332
 X500 id: drews032   lewi0486

 Operating system on which you tested your code: Linux
 CSELABS machine: apollo.cselabs.umn.edu

 GROUP INSTRUCTION:  Please make only ONLY one submission when working in a group.

***********************************************************************************************/

// NOTE: This code is the working version of the code submitted last night. Thanks for letting us resubmit our code!

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
#include <fcntl.h>

#define NAMESIZE 256
#define BUFSIZE 256
#define DATESIZE 128
#define PATHSIZE 1024
#define TOKENSIZE 100
#define BACKUP_SUFFIX ".bak"

char *file1 = 0;
char *file2 = 0;
char *file3 = 0;
int size1 = 0;
int size2 = 0;
int size3 = 0;
char linkcheck = 0;
char accessmodes[10];
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

// Copy file at path "srcfile" to destination "destfile" and return the error code
int copy_file(char* srcfile, char* destfile)
{
    char cpbuffer[BUFSIZE];
    ssize_t count;
    mode_t perms;
    int fdin, fdout;

    if  ((fdin = open(srcfile, O_RDONLY)) == -1) {
        perror("Error opening the input file:");
        return 1;
    }
    /*    printf( "Input file opened with descriptor %d \n", fdin);  */
 
    if  ((fdout = open(destfile, (O_WRONLY | O_CREAT), perms)) == -1 ) {
        perror("Error creating the output file");
        return 2;
    }
    /*    printf( "Output file opened with descriptor %d \n", fdout);  */
   
    while ((count=read(fdin, cpbuffer, BUFSIZE)) > 0) {
        if (write(fdout, cpbuffer, count) != count) { 
            perror("Error writing");
            return 3;
        }
    } 

    if (count == -1) {
        perror("Error reading the input file");
        return 4;
    }

    close(fdin);
    close(fdout);
    return 0;
}

// recurse through all directories to back them up in step 4.
// takes a source (recursepath) and a destination (backuppath)
void recurse_through_directory_backup(char* recursepath, char* backuppath)
{
    // define strings for source file and dest file
  char *srcfile, *destfile, *newbakdir, *oldsymlink, *newsymlink;
    srcfile = (char *) malloc(PATHSIZE * sizeof(char));
    destfile = (char *) malloc(PATHSIZE * sizeof(char));
    newbakdir = (char *) malloc(PATHSIZE * sizeof(char));
    oldsymlink = (char *) malloc(PATHSIZE * sizeof(char));
    newsymlink = (char *) malloc(PATHSIZE * sizeof(char));

    // define the directory variable dp
    DIR *dp;
    if ((dp = opendir(recursepath)) == NULL) {
        perror("Error while opening the directory\n");
        exit(EXIT_FAILURE);
    }

    // define the dirent for the ORIGINAL directory
    struct dirent *origdent;
    // change directory to "recursepath"
    chdir(recursepath);

    printf("Current directory: \"%s\"\n", recursepath);
    // printf("\tCurrent backup dir: \"%s\"\n", backuppath);

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
            // put "mydir/myfile" into "srcfile"
            strcpy(srcfile, recursepath);
            strcat(srcfile, "/");
            strcat(srcfile, origdent->d_name);
            // put "mydir.bak/myfile" into "destfile"
            strcpy(destfile, backuppath);
            strcat(destfile, "/");
            strcat(destfile, origdent->d_name);
            // printf("\tBacking up \"%s\"\n\t        to \"%s\"\n", srcfile, destfile);
            printf("\tBacking up \"%s\"\n", origdent->d_name);
            int cpresult = copy_file(srcfile, destfile);
            if (cpresult != 0) {
                perror("Error while copying\n");
                exit(EXIT_FAILURE);
            }
        } else if (S_ISLNK(statbuf.st_mode)) {
            // Back up the symlink by creating a new symlink in the backup folder that points to the original file
            printf("\tBacking up \"%s\" (SYMLINK)\n", origdent->d_name);
            // get symlink source path proper
            realpath(origdent->d_name, oldsymlink);
            // create symlink destination path
            strcpy(newsymlink, backuppath);
            strcat(newsymlink, "/");
            strcat(newsymlink, origdent->d_name);
            // make the link
            symlink(oldsymlink, newsymlink);
        } else { // "origdent->d_name" is a directory
            // compare directory name with "." or "..", special directories
            if (strcmp(origdent->d_name, ".") == 0 || strcmp(origdent->d_name, "..") == 0) {
                // don't go into . and ..! that's the DANGER ZONE
                // printf("\"%s\" is a SPECIAL directory\n", origdent->d_name);
            } else {
                printf("\tCreating \"%s\" and descending.\n", origdent->d_name);
                // convert "origdent->d_name" (relative directory) to absolute directory ("recursepath")
                realpath(origdent->d_name, recursepath);
                // append the new path to the backup path
                strcpy(newbakdir, backuppath);
                strcat(backuppath, "/");
                strcat(backuppath, origdent->d_name);
                // make the new directory
                mkdir(backuppath, 0755);
                // keep going through the folder
                // printf("\tNew backup path: \"%s\"\n", backuppath);
                recurse_through_directory_backup(recursepath, backuppath);
            }
      }
    }
    // GO UP A DIRECTORY, both in the source and destination directory pointers
    printf("Done with directory \"%s\", ascending.\n", recursepath);
    change_dir("..", recursepath, PATHSIZE);
    // printf("\tNow exploring: \"%s\"\n", recursepath);
    strcat(backuppath, "/..");
    // printf("\tNow backing up to: \"%s\"\n", backuppath);
}

int make_backup_directory(char *backupsrc, char *backupfolderout) {
    // strings for directories and paths
    char *backupdest, *backupold, *backupdate;
    backupdest = (char *) malloc(PATHSIZE * sizeof(char));
    backupold = (char *) malloc(PATHSIZE * sizeof(char));
    backupdate = (char *) malloc(PATHSIZE * sizeof(char));

    // copy backupsrc to backupdest and append the backup suffix
    strcpy(backupdest, backupsrc);
    strcat(backupdest, BACKUP_SUFFIX);

    printf("Backing up %s to %s\n", backupsrc, backupdest);

    // create new directory.bak, catching errors
    int errcreate = mkdir(backupdest, 0755);
    if (errcreate != 0) {
        // something went wrong
        if (errno == EEXIST) {
            // directory.bak already exists!
            // printf("%s already exists!\n", backupdest);
                
            // instantiate a string to hold path to dir.bak-2013-... and the
            // backup date suffix

            // copy backupdest to backupold and append the date suffix
            strcpy(backupold, backupdest);
            time_to_buf(backupdate, PATHSIZE);
            strcat(backupold, "-");
            strcat(backupold, backupdate);
            printf("\tRenaming old backup %s to %s\n", backupdest, backupold);
                
            // make the new backup directory already!
            // rename mydir.bak to mydir.bak-DATE
            rename(backupdest, backupold);
            // create mydir.bak from scratch
            printf("\tCreating new backup folder %s.\n", backupdest);
            mkdir(backupdest, 0755);
            printf("\t%s created successfully.\n", backupdest);
            strcpy(backupfolderout, backupdest);
            return 0;
        } else {
            printf("Unknown error while creating %s.\n", backupdest);
        }
    } else {
        printf("\t%s created successfully.\n", backupdest);
    }
}

// get access mode string from mode
char *getAccessModeString ( const mode_t mode, char mstr[] )
{
    sprintf(mstr, "---------");

    // Get user access bits
    if ( S_IRUSR & mode )  mstr[0] = '1';
    if ( S_IWUSR & mode )  mstr[1] = '1';
    if ( S_IXUSR & mode )  mstr[2] = '1';

    // Get group access bits
    if ( S_IRGRP & mode )  mstr[3] = '1';
    if ( S_IWGRP & mode )  mstr[4] = '1';
    if ( S_IXGRP & mode )  mstr[5] = '1';

    // Get other access bits
    if ( S_IROTH & mode )  mstr[6] = '1';
    if ( S_IWOTH & mode )  mstr[7] = '1';
    if ( S_IXOTH & mode )  mstr[8] = '1';
}

// recurse through a directory and do things based on the choice the user made
void recurse_through_directory(char* recursepath, int choice)
{
    // Define the directory variable dp
    DIR *dp;
    // Open directory, else print error
    if ( (dp = opendir(recursepath)) == NULL)
    {
        perror("ERROR: Unable to open directory\n");
        exit(4);
    }

    // Define directory entry structure
    struct dirent *direntry;
    // Move into next folder in directory tree
    chdir(recursepath);

    // Read each directory item
    while ((direntry = readdir(dp)) != NULL )
    {
        // Store each stat structure in statbuf
        lstat(direntry->d_name, &statbuf);
        // Check if item is not a directory
        if (!(S_ISDIR(statbuf.st_mode)))
        {
            char *filepath;
            filepath = (char *) malloc(NAMESIZE * sizeof(char));
            linkcheck = 0;
            // Check if item is a symbolic link
            if (S_ISLNK(statbuf.st_mode))
            {
                linkcheck = 1;
            }
            // Determine absolute path to item
            realpath(direntry->d_name, filepath);
            // If first option is selected
            if(choice == 1)
            {
                // Check to see if current item is largest thus far
                if(statbuf.st_size > size1)
                {
                    file3 = file2;
                    file2 = file1;
                    file1 = filepath;
                    size3 = size2;
                    size2 = size1;
                    size1 = statbuf.st_size;
                }
                // Check to see if current item is second largest thus far
                else if(statbuf.st_size > size2)
                {
                    file3 = file2;
                    file2 = filepath;
                    size3 = size2;
                    size2 = statbuf.st_size;
                }
                // Check to see if current item is third largest thus far
                else if(statbuf.st_size > size3)
                {
                    file3 = filepath;
                    size3 = statbuf.st_size;
                }
                // Else ignore item
            }
            // If second option is selected
            else if(choice == 2)
            {
                // Check to see if file size is 0 bytes
                if(statbuf.st_size == 0)
                {
                    printf("The size of file \"%s\" is %d bytes\n", filepath, statbuf.st_size);
                }
            }
            // If third option is selected
            else if(choice == 3)
            {
                // Determine permission of current item
                getAccessModeString(statbuf.st_mode, accessmodes);
                // Check to see if current item has permission 777
                if(strcmp(accessmodes, "111111111") == 0) {
                    if(linkcheck == 0)
                    {
                        printf("The permission of file \"%s\" is 777\n", filepath);
                    }
                    else
                    {
                        printf("The permission of link \"%s\", pointing to \"%s\" is 777\n", direntry->d_name, filepath);
                    }
                }
            }
        }
        // Current item is a directory
        else
        {
            // Compare directory name with "." or "..", special directories
            if (strcmp(direntry->d_name, ".") == 0 || strcmp(direntry->d_name, "..") == 0)
            {
                // Ignore directories "." and ".."
            }
            else
            {
                // Determine absolute path to directory
                realpath(direntry->d_name, recursepath);
                recurse_through_directory(recursepath, choice);
            }
        }
    }
    // Move out of current folder in directory tree
    chdir("..");
}

int main(int argc, char *argv[])
{
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

    // Determine absolute path to item
    realpath(input_dir_name, dirpath);

    // Check if item exists
    int dir_exists_error = stat(dirpath, &statbuf);
    if (dir_exists_error == -1) 
    {
        perror("ERROR: Directory does not exist\n");
        exit(1);
    } 
    else
    {
        // Check if item is a directory
        if (S_ISDIR(statbuf.st_mode))
        {
            // do nothing
        }
        else
        {
            perror("ERROR: Path is not a directory\n");
            exit(2);
        }
    }
    if(choice == 1)
    {
        printf("\nEXECUTING \"1. Find the 3 largest files in a directory\"\n");
        /********************************************************/
        /**************Function to perform choice 1**************/
        /********************************************************/
        recurse_through_directory(dirpath, 1);
        printf("The size of file \"%s\" is %d bytes\n", file1, size1);
        printf("The size of file \"%s\" is %d bytes\n", file2, size2);
        printf("The size of file \"%s\" is %d bytes\n", file3, size3);
    }
    else if(choice == 2)
    {
        printf("\nEXECUTING \"2. List all zero length files in a directory\"\n");
        /********************************************************/
        /**************Function to perform choice 2**************/
        /********************************************************/
        recurse_through_directory(dirpath, 2);
    }
    else if(choice == 3)
    {
        printf("\nEXECUTING \"3. Find all files with permission 777 in a directory\"\n");
        /********************************************************/
        /**************Function to perform choice 3**************/
        /********************************************************/
        recurse_through_directory(dirpath, 3);
    }
    else if(choice == 4)
    {
        printf("\nEXECUTING \"4. Create a backup of a directory\"\n");
        /********************************************************/
        /**************Function to perform choice 4**************/
        /********************************************************/

        char *backuppath;
        backuppath = (char *) malloc(PATHSIZE * sizeof(char));
        make_backup_directory(dirpath, backuppath);
        recurse_through_directory_backup(dirpath, backuppath);
        printf("Backup complete.\n");
    }
    else
    {
        perror("ERROR: Invalid choice\n");
        exit(3);
    }
    free(input_dir_name);
    free(dirpath);
    return 0;
}
