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
#define SERVER_PORT 5001
#define BUFFER_SIZE 1024

/* prototypes */
void die(const char *);
void pdie(const char *);

struct request_type {
    char req_command[16];
    char params[256];
};

/**********************************************************************
 * main
 **********************************************************************/

int was_msg_bye(char *msg) {
    char *msg_lower = malloc(BUFFER_SIZE);
    strcpy(msg_lower, msg);
    return (strcmp(msg_lower, "BYE") == 0);
}

void *server_instance(void *void_msgsock) {
    int rval;
    struct request_type *req;
    struct request_type *response;
    int msgsock;
    rval = malloc(sizeof(int));
    const int req_struct_size = (sizeof(char) * (256 + 16));
    req = malloc(req_struct_size);
    response = malloc(req_struct_size);
    msgsock = (int)void_msgsock;
    do {
        /* Read from client until it's closed the connection. */
        if ((rval = recv(msgsock, req, req_struct_size, 0)) < 0){
            pdie("Reading stream message");
        }
        printf("DEBUG: rval = %i\n", rval);
        if (rval == 0) {
            /* Client has closed the connection */
            fprintf(stderr, "Server: Ending connection\n");
        } else {
            printf("Server: Rec'd msg:\n");
            printf("---req_command: %s\n", req->req_command);
            printf("---params: %s\n", req->params);

            if (was_msg_bye(req->req_command)) {
                printf("Message was bye!\n");
                rval = 0;
                break;
            }

            /* Write back to client. */
            strcpy(response->req_command, DATA1);
            strcpy(response->params, DATA2);
            if (send(msgsock, response, BUFFER_SIZE, 0) < 0) {
                pdie("Writing on stream socket");
            }
        }
    } while (rval != 0);
    close(msgsock);
    free(req);
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