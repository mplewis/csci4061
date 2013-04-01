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
#define QUIT_STRING "quit"         // what the user types to exit

#define MAX_INPUT_LINE_LENGTH 2048 // Maximum length of the input pipeline command
                                   // such as "ls -l | sort -d +4 | cat "
#define MAX_CMDS_NUM   8           // maximum number of commands in a pipe list
                                   // In the above pipeline we have 3 commands
#define MAX_CMD_LENGTH 256         // A command has no more than 255 characters

FILE *logfp;

int num_cmds;
char *cmds[MAX_CMDS_NUM];             // commands split by |
int cmd_pids[MAX_CMDS_NUM];
int cmd_status[MAX_CMDS_NUM]; 
int cmd_exits[MAX_CMDS_NUM];          // exit status of each command

static sigjmp_buf jmpbuf;
int newPipeData[2];                   // the pipelines used by all processes
int oldPipeData[2];

/* WHAT UP, HERE'S HOW IT WORKS!
 *
 * fork, child execs process1
 *   stdin --> process1 --> newPipeData
 *     !! set oldPipeData = newPipeData
 *
 * fork, child execs process2
 *   process1 --> oldPipeData --> process2 --> newPipeData
 *     !! set oldPipeData = newPipeData
 *
 * fork, child execs process3
 *   process2 --> oldPipeData --> process3 --> stdout
 * 
 * thus...
 *   stdin --> process1 --> process2 --> process3 --> stdout
 *
 * that's pretty neat
 */

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
  // use the string tokenizer to split the string by '|' (pipe) characters
  cmds[count] = strtok(commandLine, "|");
  count++;
  while((cmds[count] = strtok(NULL, "|")) != NULL) {
    count++;
  }
  int j;
  //log all commands to be spawned
  for (j = 0; j < count; j++) {
    fprintf(logfp, "Command %i info: %s\n", j, cmds[j]);
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
  int ct = 0;

  // use the string tokenizer to split the string by ' ' (space) characters
  argvector[ct] = strtok(input, " ");
  ct++;
  while((argvector[ct] = strtok(NULL, " ")) != NULL) {
    ct++;
  }
  // the command has to be found in both argvector[0] and the command argument of execvp
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
		int num_cmds)
{
#ifdef DEBUG 
  // print part d: exit status and pids of all processes
  fprintf(logfp, "PID\t\tCOMMAND\t\tEXIT STATUS\n");
  int c;
  for (c = 0; c < num_cmds; c++) {
    fprintf(logfp, "%i\t\t%s\t\t%i\n", cmd_pids[c], cmds[c], cmd_exits[c]);
  }
#endif
}  

/*******************************************************************************/
/*     The create_command_process function will create a child process         */
/*     for the i'th command in the array "cmds"                                */
/*     the argument cmd_pids contains PID of all preceding command             */
/*     processes in the pipleine. This function will add at the                */
/*     i'th index the PID of the new child process.                            */
/*******************************************************************************/

// create_command_process creates a process and routes the pipes all at once
// it also checks to make sure the requested commands exist and can be run

void create_command_process (char cmd[MAX_CMD_LENGTH],    // Command line to be processed
                             int cmd_pids[MAX_CMDS_NUM],  // PIDs of pipeline processes
			     int i)                       // commmand line number being processed
{
  int child_pid;
  char cmd_only[MAX_CMD_LENGTH];
  char *cmd_args[MAX_CMD_LENGTH];

  if (!commandExists(cmd)) {
    // log bad command
    fprintf(logfp, "Command doesn't exist: %s\nKilling the pipeline\n", cmd);
    fprintf(stdout, "Command doesn't exist: %s\nKilling the pipeline\n", cmd);
    // kill pipeline
    killPipelineWhileSpawning(SIGINT, i);
    // go back to main loop without executing command
    return;
  }

  // Execute this if it isn't the last command
  if(i < (num_cmds - 1)) {
    // Create a pipe and die on errors
    if (pipe(newPipeData) == -1) {
      perror("ERROR: Failed to create pipe\n");
      exit(1);
    }
  }
  // Fork and die on errors
  if ((child_pid = fork()) == -1) {
    perror("ERROR: Could not fork\n");
    exit(-1);
  }

  // Everything after this point is post-fork.

  if(child_pid == 0) {
    // This is the child

    // If there's a previous command, attach the child's input to the past
    // command's output (oldPipeData)
    if(i > 0) {
      close(oldPipeData[1]);
      dup2(oldPipeData[0], 0);
      close(oldPipeData[0]);
    }
    // If there's a future command, attach the child's output to the future
    // command's input (newPipeData)
    if(i < (num_cmds-1)) {
      close(newPipeData[0]);
      dup2(newPipeData[1], 1);
      close(newPipeData[1]);
    }
    parse_command(cmd, cmd_only, cmd_args);
    fprintf(logfp, "Command %i info: %s\n", i, cmd); 
    execvp(cmd_only, cmd_args);

    // command execution failed, handle the error by killing the pipeline and
    // printing it to the logfile
    fprintf(logfp, "Terminating the command process while attempting to execute %s\n", cmd_only);

  } else {
    // This is the parent

    // Store the child pid in the array of child pids
    cmd_pids[i] = child_pid;

    // If there's a previous command, make sure you close the pipe (oldPipeData)
    if(i > 0) {
      close(oldPipeData[0]);
      close(oldPipeData[1]);
    }
    // If there's a future command, update oldPipeData to match the newly-created
    // pipe so it's ready for the next command
    if(i < (num_cmds - 1)) {
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
    int status;
    fprintf(logfp, "waiting...");
    waitpid(cmd_pids[j], NULL, 0);
    cmd_exits[j] = WEXITSTATUS(status);
    j++;
    fprintf(logfp, "Process id %d finished\n", cmd_pids[j]);
    fprintf(logfp, "Process id %d finished with exit status %d\n", cmd_pids[j], cmd_exits[j]);
  }
  fprintf(logfp, "\n");
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
  siglongjmp(jmpbuf, 1);
}

// killPipelineWhileSpawning is used only when creating a bad process that must
// be killed. num_cmds doesn't work because it has been incremented but the child
// pid has not been stored, meaning kill() kills pid 0, meaning it kills piper too.
// to kill just the children that have been spawned, we have to kill everything but
// cmd_pids[j]
void killPipelineWhileSpawning( int signum , int i) {
  int j = 0;
  while(j < i - 1) {
    kill(cmd_pids[j], SIGKILL);
    j++;
  }
  siglongjmp(jmpbuf, 1);
}

// commandExists(cmd) checks if cmd exists by running 'which [cmd]'
// if 'which [cmd]' returns non-zero, then cmd does not exist

int commandExists(char test_cmd[MAX_CMD_LENGTH]) {
  char whichStr[MAX_CMD_LENGTH];
  char input_command[MAX_CMD_LENGTH];
  strcpy(input_command, test_cmd);
  char *cmd_alone;
  cmd_alone = strtok(input_command, " ");
  strcpy(whichStr, "which ");
  strcat(whichStr, cmd_alone);
  strcat(whichStr, " >/dev/null 2>&1");
  printf("CMD %s\n", whichStr);
  return (system(whichStr) == 0);
}

/********************************************************************************/

int main(int ac, char *av[]){

  int i;
  
  // return an error if someone tries to pass arguments into the program
  // at the command line
  if (ac > 1){
    printf("\nIncorrect use of parameters\n");
    printf("USAGE: %s\n", av[0]);
    exit(1);
  }

  // set up signal handler for ctrl-c to kill only the pipeline processes
  sigsetjmp(jmpbuf, 1);

  logfp = fopen("LOGFILE", "w");

  while (1) {
    char pipeCommand[MAX_INPUT_LINE_LENGTH];
    // QUIT_STRING is what the user types to exit
    char* terminator = QUIT_STRING;

    // set sigint to default behavior (interrupt the program)
    signal(SIGINT, SIG_DFL);
    fflush(stdout);
    printf("Give a list of pipe commands: ");
    gets(pipeCommand);
    printf("You entered: %s\n", pipeCommand);
    // if the user types QUIT_STRING, then quit
    if (strcmp(pipeCommand, terminator) == 0) {
      fflush(logfp);
      fclose(logfp);
      printf("Goodbye!\n");
      exit(0);
    }

    // parse the command line and count how many separate commands
    // are to be piped together
    num_cmds = parse_command_line(pipeCommand, cmds);

    // set up signal handler to handle ctrl-c
    signal(SIGINT, killPipeline); 

    for(i = 0; i < num_cmds; i++){
      // create the command process, checking to make sure each program exists
      create_command_process(cmds[i], cmd_pids, i);
    }
    fprintf(logfp, "Number of commands from the input: %d\n\n", num_cmds);

    // debug info to logfile
    print_info(cmds, cmd_pids, cmd_status, num_cmds);

    // wait for pipeline to close
    waitPipelineTermination();

    // debug info to logfile after pipeline closes
    print_info(cmds, cmd_pids, cmd_status, num_cmds);

    }
}
