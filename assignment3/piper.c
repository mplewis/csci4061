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

#define DEBUG

#define MAX_INPUT_LINE_LENGTH 2048 // Maximum length of the input pipeline command
                                   // such as "ls -l | sort -d +4 | cat "
#define MAX_CMDS_NUM   8           // maximum number of commands in a pipe list
                                   // In the above pipeline we have 3 commands
#define MAX_CMD_LENGTH 256         // A command has no more than 255 characters

FILE *logfp;

int num_cmds = 0;
char *cmds[MAX_CMDS_NUM];          // commands split by |
int cmd_pids[MAX_CMDS_NUM];
int cmd_status[MAX_CMDS_NUM]; 

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
  cmds[0] = strtok(commandLine, "|");
  num_cmds++;
  while((cmds[num_cmds] = strtok(NULL, "|")) != NULL) {
    num_cmds++;
  }
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
  int j = 0;
  argvector[j] = strtok(input, " ");
  j++;
  while((argvector[j] = strtok(NULL, " ")) != NULL) {
    j++;
  }
  command = argvector[0];
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


void create_command_process (char cmds[MAX_CMDS_NUM],   // Command line to be processed
                             int cmd_pids[MAX_CMDS_NUM],  // PIDs of pipeline processes
		                         int i)                       // commmand line number being processed
{
  int child_pid = fork();
  if (child_pid) {
    // this process is the parent, store the child pid in the array
    cmd_pids[i] = child_pid
  } else {
    // this process is the child
    char cmd_with_args[MAX_CMD_LENGTH] = cmds[i];
    char cmd_only[MAX_CMD_LENGTH];
    char *cmd_args[MAX_CMD_LENGTH];

    // parse the cmd_with_args into cmd_only and cmd_args
    parse_command(cmd_with_args, cmd_only, cmd_args);

    // execute the command
    execvp(cmd_only, cmd_with_args);

    // if this point is reached, execvp has failed; print an error to console and die
    fprintf(stderr, "ERROR: failed to execute %s\n", cmds[0]);
    exit(-1);
  }
}


/********************************************************************************/
/*   The function waitPipelineTermination waits for all of the pipeline         */
/*   processes to terminate.                                                    */
/********************************************************************************/

void waitPipelineTermination () {


}

/********************************************************************************/
/*  This is the signal handler function. It should be called on CNTRL-C signal  */
/*  if any pipeline of processes currently exists.  It will kill all processes  */
/*  in the pipeline, and the piper program will go back to the beginning of the */
/*  control loop, asking for the next pipe command input.                       */
/********************************************************************************/

void killPipeline( int signum ) {


}

/********************************************************************************/

int main(int ac, char *av[]){

  int i,  pipcount;
  //check usage
  if (ac > 1){
    printf("\nIncorrect use of parameters\n");
    printf("USAGE: %s \n", av[0]);
    exit(1);
  }

  /* Set up signal handler for CNTRL-C to kill only the pipeline processes  */
  /* if(sigaction(SIGINT, NULL, &act) == -1) {
    perror("ERROR in getting old handler for SIGINT")
      }
  else {
    if(act.sa_handler == SIG_DFL) {
      act.sa_handler = SIG_IGN;
      if(sigaction(SIGINT, &act, NULL) == -1) {
	perror("Error in setting ignore for SIGINT");
      }
      killPipeline(SIGINT);
    }
  }
  */
  logfp =  fopen("LOGFILE", "w");


  while (1) {
     signal(SIGINT, SIG_DFL ); 
     pipcount = 0;

     /*  Get input command file anme form the user */
     char pipeCommand[MAX_INPUT_LINE_LENGTH];

     fflush(stdout);
     printf("Give a list of pipe commands: ");
     gets(pipeCommand); 
     char* terminator = "quit";
     printf("You entered : list of pipe commands  %s\n", pipeCommand);
     if ( strcmp(pipeCommand, terminator) == 0  ) {
        fflush(logfp);
        fclose(logfp);
        printf("Goodbye!\n");
        exit(0);
     }  

    num_cmds = parse_command_line( pipeCommand, cmds);

    /*  SET UP SIGNAL HANDLER  TO HANDLE CNTRL-C                         */
    signal(SIGINT, killPipeline); 

    /*  num_cmds indicates the number of command lines in the input file */

    /* The following code will create a pipeline of processes, one for   */
    /* each command in the given pipe                                    */
    /* For example: for command "ls -l | grep ^d | wc -l "  it will      */
    /* create 3 processes; one to execute "ls -l", second for "grep ^d"  */
    /* and the third for executing "wc -l"                               */
   
    for(i=0;i<num_cmds;i++){
         /*  CREATE A NEW PROCCES EXECUTTE THE i'TH COMMAND    */
         /*  YOU WILL NEED TO CREATE A PIPE, AND CONNECT THIS NEW  */
         /*  PROCESS'S stdin AND stdout  TO APPROPRIATE PIPES    */  
         create_command_process (cmds[i], cmd_pids, i);
    }

    print_info(cmds, cmd_pids, cmd_status, num_cmds);

    waitPipelineTermination();

    print_info(cmds, cmd_pids, cmd_status, num_cmds);

  }
} //end main

/*************************************************/
