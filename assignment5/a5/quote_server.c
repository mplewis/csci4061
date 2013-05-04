/**********************************************************************
 * struct_server.c --- Demonstrate the core workings of a simple server
 *              - compile/run on linux
 *              - note that server does not exit on its own, you 
 *                must kill it yourself.
 *              - Do NOT leave this running on itlabs machines
 *              - Note: some code borrowed from web, source unknown
 **********************************************************************/


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#define DATA1 "Server says:"
#define DATA2 "All your base are belong to us..."
#define TRUE 1
#define SERVER_PORT 6789
#define BUFFER_SIZE 1024

/* prototypes */
void die(const char *);
void pdie(const char *);

/*
Converts a string to all lowercase, in-place.
Params:
    char *str: String to convert to lowercase.
*/
void str_lower(char *str) {
    int len = strlen(str);
    for (int i = 0; i < len; i++) {
        str[i] = tolower(str[i]);
    }
}

/*
Params:
Returns:
*/
}

/*
Message server instance. Use me as a pthread!
Pass in the file descriptor of an open socket.
*/
void *server_instance(void *void_msgsock) {
    int rval;
    char *request;
    char *response;
    int msgsock;
    rval = malloc(sizeof(int));
    request = malloc(BUFFER_SIZE);
    response = malloc(BUFFER_SIZE);
    msgsock = (int)void_msgsock;
    do {
        /* Read from client until it's closed the connection. */
        if ((rval = recv(msgsock, request, BUFFER_SIZE, 0)) < 0){
            pdie("Reading stream message");
        }
        printf("DEBUG: rval = %i\n", rval);
        if (rval == 0) {
            /* Client has closed the connection */
            fprintf(stderr, "Server: Ending connection\n");
        } else {
            printf("Server: Rec'd msg:\n");
            printf("    %s\n", request);

            /* Write back to client. */
            strcpy(request, DATA1);
            if (send(msgsock, response, BUFFER_SIZE, 0) < 0) {
                pdie("Writing on stream socket");
            }
        }
    } while (rval != 0);
    close(msgsock);
    free(request);
    free(response);
    printf("Thread here, signing off. Good bye.\n");
}

int main(void) {

    int sock;                    /* fd for main socket */
    int msgsock;                 /* fd from accept return */
    struct sockaddr_in server;   /* socket struct for server connection */
    struct sockaddr_in client;   /* socket struct for client connection */
    int clientLen;               /* returned length of client from accept() */
    int rval;                    /* return value from read() */

    /* Open a socket */
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        pdie("Opening stream socket");
    }

    /* Fill out inetaddr struct */
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(SERVER_PORT);
    
    /* Bind */
    if (bind(sock, (struct sockaddr *) &server, sizeof(server))){
        pdie("Binding stream socket");
    }

    printf("Server: Socket has port %hu\n", ntohs(server.sin_port));

    /* Listen w/ max queue of 5 */
    listen(sock, 5);

    /* Loop, waiting for client connections. */
    /* This is an interactive server. */
    while (TRUE) {
        clientLen = sizeof(client);
        if ((msgsock = accept(sock, (struct sockaddr *) &client, &clientLen)) == -1) {
            pdie("Accept");
        } else {
            int *msgsock_pointer = msgsock;
            pthread_t tid;
            pthread_create(&tid, NULL, server_instance, (void *)msgsock_pointer);
        }   /* else */
    }
    exit(0);
}


/**********************************************************************
 * pdie --- Call perror() to figure out what's going on and die.
 **********************************************************************/

void pdie(const char *mesg) {

    perror(mesg);
    exit(1);
}


/**********************************************************************
 * die --- Print a message and die.
 **********************************************************************/

void die(const char *mesg) {

    fputs(mesg, stderr);
    fputc('\n', stderr);
    exit(1);
}