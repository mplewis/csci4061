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


char *cmds[MAX_CMDS_NUM];
int cmd_pids[MAX_CMDS_NUM];
int cmd_status[MAX_CMDS_NUM];

int splitCmds(char *multipleCmds)
{
	int num_cmds = 0;
	cmds[0] = strtok(multipleCmds, "|");
	printf("%s\n", cmds[0]);
	num_cmds++;
	while ((cmds[num_cmds] = strtok(NULL, "|")) != NULL) {
		printf("%s\n", cmds[num_cmds]);
		num_cmds++;
	}
	return num_cmds;
}

int main()
{
	char pipeCommand[MAX_INPUT_LINE_LENGTH];
	printf("Give me some pipe commands: ");
	gets(pipeCommand);
	printf("Commands: <%s>\n", pipeCommand);

	printf("Number of commands: %i\n", splitCmds(pipeCommand));

	return 0;
}