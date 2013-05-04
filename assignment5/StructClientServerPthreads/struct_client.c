/**********************************************************************
 * struct_client.c --- Client to connect to quote_server
 *                     - change SERVER_PORT value to that of your server
 **********************************************************************/

/* Requisite includes */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define SERVER_PORT 5001 /* CHANGE THIS TO THE PORT OF YOUR SERVER */
#define BUFFER_SIZE 1024

struct request_type {
  char req_command[16];
  char params[256];
};

/**********************************************************************
 * main
 **********************************************************************/

int main(int argc, char *argv[]) {

  int sock;                    /* fd for socket connection */
  struct sockaddr_in server;   /* Socket info. for server */
  struct hostent *host;        /* Server info */  
  char selection[BUFFER_SIZE]; /* User's input */
  int finished = 0;  

  struct request_type req;
  struct request_type response;
  
  if (argc != 2) {
    printf("Usage: %s address\n", argv[0]);
  }
     
  /* Fill out sockaddr struct */
  server.sin_family = AF_INET;
  if ((host = gethostbyname(argv[1])) == NULL){
    printf("%s: unknown host '%s'\n", argv[0], argv[1]);
    exit(1);
  }
  server.sin_family = host->h_addrtype;
  memcpy((char *) &server.sin_addr.s_addr, host->h_addr_list[0], host->h_length);
  server.sin_port = htons((u_short) SERVER_PORT);
  
  /* Create socket */
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    printf("%s: Unable to open socket\n",argv[0]);
    perror("Aborting");
    exit(1);
  }

  /* Try to connect */
  if (connect(sock, (struct sockaddr *) &server, sizeof(server)) < 0){
    printf("%s: Unable to connect to %s\n", argv[0], argv[1]);
    perror("Aborting");
    exit(1);
  } else {
    printf("\nConnection to the quote server established\n");
  }

  while (!finished){
    
    memset((void *)response.req_command, '\0', 16);
    memset((void *)response.params, '\0', 256);
    memset((void *)req.req_command, '\0', 16);
    memset((void *)req.params, '\0', 256);

    /* Get input from user */
    printf("Options:\n");
    printf("  Press <ENTER> for any quote\n");
    printf("  'list' for a list of categories\n");
    printf("  <category name> for a quote from a specific category\n");
    printf("  'bye' to quit\n\n");
    do {
      printf("Enter your selection: ");
      fflush(stdout);
      fgets(selection, BUFFER_SIZE, stdin);
    } while (selection == NULL);
    
    printf("\n");
    /* Parse user's selection */
    if (strncmp(selection, "bye\n",4) == 0){
      sprintf(req.req_command,"BYE");
      finished = 1;
    } else if (strncmp(selection, "\n", 1) == 0){
      sprintf(req.req_command,"QUOTE");
      sprintf(req.params, "ANY");
    } else if (strncmp(selection, "list\n", 5) == 0){
      sprintf(req.req_command,"LIST");
    } else {
      sprintf(req.req_command,"QUOTE");
      sprintf(req.params,"%s\n", selection);
    }
    
    /* Send user's request to server. */
    if (send(sock, &req, sizeof(req),0) < 0) {
      printf("%s: Error writing on stream socket", argv[0]);
      perror("Aborting");
      close(sock);
      exit(1);
    }
    
    if (!finished){
      /* Get response from server */
      if (recv(sock, &response, sizeof(response),0) < 0){
	printf("%s: Error reading from stream socket", argv[0]);
	perror("Aborting");
	close(sock);
	exit(1);
      }
      
      /* Print response */
      printf("Response from server:\n");
      printf("---req_command: %s\n",response.req_command);
      printf("---params: %s\n", response.params);
    }

  } /* end while(!finished) */
  
  close(sock);
  printf("Connection to server terminated. Exiting...\n\n");
  exit(0);
 
} /* End of main() */

/*********************************************************************/




