#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#define BUFSIZE 64
#define STR_TO_SEARCH "   ps aux | grep git | tail "

int countPipes(char *strToSearch)
{
	int count = 0;
	for (int i = 0; i < strlen(strToSearch); i++) {
		if (strToSearch[i] == '|')
			count++;
	}
	return count;
}

char *ptrStrCmd;

int main() {
	char strCmd[BUFSIZE] = STR_TO_SEARCH;
	ptrStrCmd = strCmd;

	printf("Message:         <%s>\n", ptrStrCmd);
	printf("Number of pipes: %i\n", countPipes(ptrStrCmd));
}