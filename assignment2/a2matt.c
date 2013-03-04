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

char *file1 = 0;
char *file2 = 0;
char *file3 = 0;
int size1 = 0;
int size2 = 0;
int size3 = 0;
char accessmodes[10];
struct stat statbuf;

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
      if (!(S_ISDIR(statbuf.st_mode))) {
	char *filepath;
	filepath = (char *) malloc(NAMESIZE * sizeof(char));
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
	      printf("The permission of file \"%s\" is 777\n", filepath);
	    }
	  }
	// If the fourth item is selected
	else if(choice == 4)
	  {
	}

      }
      // Current item is a directory
      else
	{ //
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
	recurse_through_directory(dirpath, 4);
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

