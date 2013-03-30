#define MAX_CMD_LENGTH 256

char cmd1[MAX_CMD_LENGTH];
char cmd2[MAX_CMD_LENGTH];
char cmd3[MAX_CMD_LENGTH];

void main()
{
	strcpy(cmd1, "grep");
	strcpy(cmd2, "ls");
	strcpy(cmd3, "wc");
	printf("Command 1: %s\n", cmd1);
	printf("Command 2: %s\n", cmd2);
	printf("Command 3: %s\n", cmd3);
}