/***********************************************************************************************

 CSci 4061 Spring 2013
 Assignment# 3: Piper program for executing pipe commands 

 Student name: Matthew Lewis
 Student ID:   4237332

 Student name: Christian Drews
 Student ID:   4368668   

 X500 id: lewi0486, drews032

 Operating system on which you tested your code: Linux
 CSELABS machine: apollo.cselabs.umn.edu

 GROUP INSTRUCTION:  Please make only ONLY one submission when working in a group.
***********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <setjmp.h>

#define DEBUG

#define MAX_INPUT_LINE_LENGTH 2048 // Maximum length of the input pipeline command
                                   // such as "ls -l | sort -d +4 | cat "
#define MAX_CMDS_NUM   8           // maximum number of commands in a pipe list
                                   // In the above pipeline we have 3 commands
#define MAX_CMD_LENGTH 256         // A command has no more than 255 characters

FILE *logfp;

int num_cmds;
char *cmds[MAX_CMDS_NUM];          // commands split by |
int cmd_pids[MAX_CMDS_NUM];
int cmd_status[MAX_CMDS_NUM]; 

static sigjmp_buf jmpbuf;
int newPipeData[2];                   // the pipeline used by everything
int oldPipeData[2];

/*******************************************************************************/
/*   The function parse_command_line will take a string such as
     ls -l | sort -d +4 | cat | wc
     given in the char array commandLine, and it will separate out each pipe
     command and store pointer to the command strings in array "cmds"
     For example:
     cmds[0]  will pooint to string "ls -l"
     cmds[1] will point to string "sort -d +4"
     cmds[2] will point to string "cat"
     cmds[3] will point to string "wc"

     This function will write to the LOGFILE above information.
*/
/*******************************************************************************/

int parse_command_line (char commandLine[MAX_INPUT_LINE_LENGTH], char* cmds[MAX_CMDS_NUM])
{
  int count = 0;
  cmds[count] = strtok(commandLine, "|");
  count++;
  while((cmds[count] = strtok(NULL, "|")) != NULL) {
    count++;
  }
  return count;
}

/*******************************************************************************/
/*  parse_command takes command such as  
    sort -d +4
    It parses a string such as above and puts command program name "sort" in
    argument array "cmd" and puts pointers to all command arguments in the argvector.
    It will return  argvector as follows
    command "cmd" will be "sort"
    argvector[0] will be "sort"
    argvector[1] will be "-d"
    argvector[2] will be "+4"
/
/*******************************************************************************/

void parse_command(char input[MAX_CMD_LENGTH],
                   char command[MAX_CMD_LENGTH],
                   char *argvector[MAX_CMD_LENGTH]){
  int count = 0;
  argvector[count] = strtok(input, " ");
  count++;
  while((argvector[count] = strtok(NULL, " ")) != NULL) {
    count++;
  }
  strcpy(command, argvector[0]);
}


/*******************************************************************************/
/*  The function print_info will print to the LOGFILE information about all    */
/*  processes  currently executing in the pipeline                             */
/*  This printing should be enabled/disabled with a DEBUG flag                 */
/*******************************************************************************/

void print_info(char* cmds[MAX_CMDS_NUM],
		int cmd_pids[MAX_CMDS_NUM],
		int cmd_stat[MAX_CMDS_NUM],
		int num_cmds) {

#ifdef DEBUG
  fprintf(logfp, "Commands: %d, Command PIDs: %d, Command Stats: %d, Number of Commands: %d \n", cmds, cmd_pids, cmd_stat, num_cmds); 
#endif

}  



/*******************************************************************************/
/*     The create_command_process function will create a child process         */
/*     for the i'th command in the array "cmds"                                */
/*     the argument cmd_pids contains PID of all preceding command             */
/*     processes in the pipleine. This function will add at the                */
/*     i'th index the PID of the new child process.                            */
/*******************************************************************************/


void create_command_process (char cmd[MAX_CMD_LENGTH],   // Command line to be processed
                             int cmd_pids[MAX_CMDS_NUM],  // PIDs of pipeline processes
			     int i)                       // commmand line number being processed
{
  int child_pid;
  char cmd_only[MAX_CMD_LENGTH];
  char *cmd_args[MAX_CMD_LENGTH];

  if(i < (num_cmds-1)) {
    if (pipe(newPipeData) == -1) {
      perror("ERROR: Failed to create pipe\n");
      exit(1);
    }
  }
  if ((child_pid = fork()) == -1) {
    perror("ERROR: Could not fork\n");
    exit(-1);
  }

  if(child_pid == 0) { // This is the child
    if(i > 0) { // Is there a previous command?
      close(oldPipeData[1]);
      dup2(oldPipeData[0], 0);
      close(oldPipeData[0]);
    }
    if(i < (num_cmds-1)) { // Is there a future command?
      close(newPipeData[0]);
      dup2(newPipeData[1], 1);
      close(newPipeData[1]);
    }
    parse_command(cmd, cmd_only, cmd_args);
    execvp(cmd_only, cmd_args);
  }
  else { // This is the parent
    cmd_pids[i] = child_pid;
    if(i > 0) { // Is there a previous command?
      close(oldPipeData[0]);
      close(oldPipeData[1]);
    }
    if(i < (num_cmds-1)) { // Is there a future command?
      oldPipeData[0] = newPipeData[0];
      oldPipeData[1] = newPipeData[1];
    }
  }
}

/********************************************************************************/
/*   The function waitPipelineTermination waits for all of the pipeline         */
/*   processes to terminate.                                                    */
/********************************************************************************/

void waitPipelineTermination () {
  int j = 0;
  while(j < num_cmds) {
    waitpid(cmd_pids[j], NULL, 0);
    j++;
  }
}

/********************************************************************************/
/*  This is the signal handler function. It should be called on CNTRL-C signal  */
/*  if any pipeline of processes currently exists.  It will kill all processes  */
/*  in the pipeline, and the piper program will go back to the beginning of the */
/*  control loop, asking for the next pipe command input.                       */
/********************************************************************************/

void killPipeline( int signum ) {
  int j = 0;
  while(j < num_cmds) {
    kill(cmd_pids[j], SIGKILL);
    j++;
  }
  printf("\nPipeline Killed\n");
  siglongjmp(jmpbuf, 1);
}

/********************************************************************************/

int main(int ac, char *av[]){

  int i;
  //check usage
  if (ac > 1){
    printf("\nIncorrect use of parameters\n");
    printf("USAGE: %s \n", av[0]);
    exit(1);
  }

  /* Set up signal handler for CNTRL-C to kill only the pipeline processes  */
  sigsetjmp(jmpbuf, 1);

  logfp = fopen("LOGFILE", "w");

  while (1) {
    signal(SIGINT, SIG_DFL);

    char pipeCommand[MAX_INPUT_LINE_LENGTH];

    fflush(stdout);
    printf("Give a list of pipe commands: ");
    gets(pipeCommand); 
    char* terminator = "quit";
    printf("You entered: %s\n", pipeCommand);
    if (strcmp(pipeCommand, terminator) == 0) {
      fflush(logfp);
      fclose(logfp);
      printf("Goodbye!\n");
      exit(0);
    }  

    // parse the command line and count the commands
    num_cmds = parse_command_line(pipeCommand, cmds);

    // set up signal handler to handle ctrl-c
    signal(SIGINT, killPipeline); 

    for(i = 0; i < num_cmds; i++){
      // for each command, create a new process and pipe the processes together
      create_command_process(cmds[i], cmd_pids, i);
    }

    // debug info to logfile
    print_info(cmds, cmd_pids, cmd_status, num_cmds);

    // wait for pipeline to close
    waitPipelineTermination();

    // debug info to logfile after pipeline closes
    print_info(cmds, cmd_pids, cmd_status, num_cmds);

    }
}