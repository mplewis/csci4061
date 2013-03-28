#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#define BUFSIZE 64
#define STR_TO_STRIP "   Hello world!     "

size_t trimwhitespace(char *out, size_t len, const char *str)
{
	if(len == 0)
		return 0;

	const char *end;
	size_t out_size;

	// Trim leading space
	while(isspace(*str)) str++;

	if(*str == 0)  // All spaces?
	{
		*out = 0;
		return 1;
	}

	// Trim trailing space
	end = str + strlen(str) - 1;
	while(end > str && isspace(*end)) end--;
	end++;

	// Set output size to minimum of trimmed string length and buffer size minus 1
	out_size = (end - str) < len-1 ? (end - str) : len-1;

	// Copy trimmed string and add null terminator
	memcpy(out, str, out_size);
	out[out_size] = 0;

	return out_size;
}

char *ptrStrIn, *ptrStrStripped;

int main()
{
	char strIn[BUFSIZE] = STR_TO_STRIP;
	ptrStrIn = strIn;
	char strStripped[BUFSIZE] = {0};
	ptrStrStripped = strStripped;
	
	trimwhitespace(ptrStrStripped, BUFSIZE, ptrStrIn);

	printf("Message:  <%s>\n", ptrStrIn);
	printf("Stripped: <%s>\n", ptrStrStripped);
	
	return 0;
}