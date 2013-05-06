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
#include <ctype.h>
#include <time.h>

#define LOG_FILE_LOC          "server.log"
#define QUOTE_CONFIG_FILE_LOC "config.txt"
#define MAX_QUOTE_FILES       16
#define MAX_LINE_LEN          1024
#define BUFFER_SIZE           1024
#define SERVER_PORT 6789

FILE *fp;
FILE *log_file;

FILE *file_pointers[MAX_QUOTE_FILES] = {0};
char *cat_names[MAX_QUOTE_FILES] = {0};
char *cat_file_locs[MAX_QUOTE_FILES] = {0};
int num_cats;
int last_line_char;

/* prototypes */
void die(const char *);
void pdie(const char *);

/*
Reads the config file and picks it apart into category names and
    category text file locations, then puts those into their own arrays.
Params:
    int max_num_cats: The maximum number of quote files through which to
        iterate.
    char **cat_names: The array into which to place category names.
    char **cat_file_locs: The array into which to place the locations of
        category text files.
Returns:
    (int) The number of categories found.
*/
int init_cats_from_config(int max_num_cats, char **cat_names,
                          char **cat_file_locs) {
    if ((fp = fopen(QUOTE_CONFIG_FILE_LOC, "r")) == NULL) {
        printf("Failure opening %s\n", QUOTE_CONFIG_FILE_LOC);
        exit(1);
    }
    int num_categories = 0;
    char *token;
    char *line_from_cfg = malloc(sizeof(char) * MAX_LINE_LEN);
    while (fgets(line_from_cfg, MAX_LINE_LEN, fp) != NULL) {
        if (num_categories == max_num_cats) {
            break;
        }
        // strip newline char from line
        int last_line_char = strlen(line_from_cfg) - 1;
        if (line_from_cfg[last_line_char] == '\n') {
            line_from_cfg[last_line_char] = '\0';
        }
        token = NULL;
        token = strtok(line_from_cfg, ": ");
        int token_count = 0;
        while (token != NULL) {
            // copy and store tokens in the proper array
            char *token_copy = malloc(sizeof(char) * BUFFER_SIZE);
            // tokenize the line by the separator ": "
            strcpy(token_copy, token);
            // the first token is the category name, the second token is the
            // category text file location
            if (token_count == 0) {
                cat_names[num_categories] = token_copy;
            } else if (token_count == 1) {
                cat_file_locs[num_categories] = token_copy;
            }
            token = strtok(NULL, ": ");
            token_count++;
        }
        num_categories++;
    }
    return num_categories;
}

/*
Opens file pointers for each quote category text file.
Params:
    FILE **file_pointers: An array of file pointers. Open file pointers will
        be placed into this array in the same order as the text file locations
        are found in cat_file_locs.
    int num_categories: The number of category text file locations through which
        to iterate in **cat_file_locs.
    char **cat_file_locs: An array of char*. Each char* contains a file location
        to which a file pointer should be opened.
*/
void open_fps_from_cat_file_locs(FILE **file_pointers, int num_categories,
                                 char **cat_file_locs) {
    for (int i = 0; i < num_categories; i++) {
        if ((file_pointers[i] = fopen(cat_file_locs[i], "r")) == NULL) {
            printf("Failure opening %s\n", cat_file_locs[i]);
            exit(1);
        }
    }
}

/*
Gets the next quote from the file pointer of a quote category text file.
Params:
    char *quote_text: The body of the quote will be placed into this char*.
    char *quote_author: The author of the quote will be placed into this char*.
    FILE *cat_file_pointer: The file pointer to the quote category text file
        from which to read quotes and quote authors.
*/
void get_next_quote_from_cat_file(char *quote_text, char *quote_author,
                                  FILE *cat_file_pointer) {
    int last_line_char;
    char *quote_line = malloc(sizeof(char) * MAX_LINE_LEN);

    // check if fp is null
    if (cat_file_pointer == NULL) {
        printf("FP is null\n");
        return;
    }

    if (fgets(quote_line, MAX_LINE_LEN, cat_file_pointer) == NULL) {
        rewind(cat_file_pointer);
    }
    // strip newline char from line
    last_line_char = strlen(quote_line) - 1;
    if (quote_line[last_line_char] == '\n') {
        quote_line[last_line_char] = '\0';
    }
    strcpy(quote_text, quote_line);

    if (fgets(quote_line, MAX_LINE_LEN, cat_file_pointer) == NULL) {
        rewind(cat_file_pointer);
    }
    // strip newline char from line
    last_line_char = strlen(quote_line) - 1;
    if (quote_line[last_line_char] == '\n') {
        quote_line[last_line_char] = '\0';
    }
    strcpy(quote_author, quote_line);
}

/*
Closes all file pointers in an array of file pointers.
Params:
    int num_file_pointers: The number of file pointers found in the array.
    FILE **file_pointers: The array containing file pointers to be closed.
*/
void close_all_fps(int num_file_pointers, FILE **file_pointers) {
    for (int i = 0; i < num_file_pointers; i++) {
        fclose(file_pointers[i]);
    }
}

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
Converts a string to all uppercase, in-place.
Params:
    char *str: String to convert to uppercase.
*/
void str_upper(char *str) {
    int len = strlen(str);
    for (int i = 0; i < len; i++) {
        str[i] = toupper(str[i]);
    }
}

/*
Given a category name and the array of category file pointers, returns the body
    of a quote and its author. Also accepts "ANY". Returns -1 if category does
    not exist.
Params:
    char *quote_text: The body of the quote from the selected category.
    char *quote_author: The author of the quote from the selected category.
    int num_cats: The number of different categories in the arrays of
        categories.
    char **cat_names: The array of category names to search for `category`.
    FILE **cat_file_ptrs: The array of category file pointers.
    char *category: A string containing the name of the category from which
        quotes should be retrieved.
Returns:
    (int) 0 if category was found in cat_names, implying quote_text and
        quote_author were updated. -1 if category was not found.
*/
int get_quote_from_specific_category(char *quote_text, char *quote_author,
                                     int num_cats, char **cat_names,
                                     FILE **cat_file_ptrs, char *category) {
    char *str_to_cmp_a = malloc(sizeof(char) * MAX_LINE_LEN);
    strcpy(str_to_cmp_a, category);
    str_lower(str_to_cmp_a);
    int cat_found = 0;
    if (strcmp(str_to_cmp_a, "any") == 0) {
        cat_found = 1;
        // pick a random category
        int cat_num = (rand() % num_cats);
        FILE *cat_fp = cat_file_ptrs[cat_num];
        get_next_quote_from_cat_file(quote_text, quote_author, cat_fp);
    } else {
        char *str_to_cmp_b = malloc(sizeof(char) * MAX_LINE_LEN);
        for (int i = 0; i < num_cats; i++) {
            if (cat_found == 1) {
                break;
            }
            strcpy(str_to_cmp_b, cat_names[i]);
            str_lower(str_to_cmp_b);
            if (strcmp(str_to_cmp_a, str_to_cmp_b) == 0) {
                cat_found = 1;
                FILE *cat_fp = cat_file_ptrs[i];
                get_next_quote_from_cat_file(quote_text, quote_author, cat_fp);
            }
        }
        free(str_to_cmp_b);
    }
    free(str_to_cmp_a);
    if (cat_found == 0) {
        return -1;
    } else {
        return 0;
    }
}

/*
Message server instance. Use me as a pthread!
Pass in the file descriptor of an open socket.
*/
void *server_instance(void *void_msgsock) {
    int rval;
    int msgsock;
    int was_bye;
    char *request;
    char *response;
    char *quote_cat;
    char *quote_text;
    char *quote_author;
    msgsock = (int)void_msgsock;
    request = malloc(BUFFER_SIZE);
    response = malloc(BUFFER_SIZE);
    quote_cat = malloc(BUFFER_SIZE);
    quote_text = malloc(BUFFER_SIZE);
    quote_author = malloc(BUFFER_SIZE);
    do {
        /* Read from client until it's closed the connection. */
        if ((rval = recv(msgsock, request, BUFFER_SIZE, 0)) < 0) {
            pdie("Reading stream message");
        }
        if (rval == 0) {
            /* Client has closed the connection */
            fprintf(stderr, "Server: Ending connection\n");
        } else {
            printf("Server: Rec'd msg:\n");
            printf("    \"%s\"\n", request);

            was_bye = 0;
            if (strcmp(request, "GET: LIST\n") == 0) {
                printf("Detected request for category list\n");
                strcpy(response, "Categories:\n");
                for (int i = 0; i < num_cats; i++) {
                    strcat(response, cat_names[i]);
                    strcat(response, "\n");
                }
            } else if (strcmp(request, "BYE\n") == 0) {
                printf("Bye-bye!\n");
                was_bye = 1;
            } else {
                str_upper(request);
                // find the last colon in the request string
                quote_cat = strrchr(request, ':');
                quote_cat += 2;
                // strip newline char from line
                int last_line_char = strlen(quote_cat) - 1;
                if (quote_cat[last_line_char] == '\n') {
                    quote_cat[last_line_char] = '\0';
                }
                printf("Detected quote category: \"%s\"\n", quote_cat);
                if (get_quote_from_specific_category(quote_text, quote_author,
                                                     num_cats, cat_names,
                                                     file_pointers, quote_cat)
                    == -1) {
                    printf("Category %s not found\n", quote_cat);
                    strcpy(response, "Category ");
                    strcat(response, quote_cat);
                    strcat(response, " not found\n");
                } else {
                    strcpy(response, quote_text);
                    strcat(response, "\n");
                    strcat(response, quote_author);
                    strcat(response, "\n");
                }
            }

            if (was_bye == 0) {
                /* Write back to client. */
                if (send(msgsock, response, BUFFER_SIZE, 0) < 0) {
                    pdie("Writing on stream socket");
                }
            }
        }
    } while (rval != 0);
    close(msgsock);
    free(request);
    free(response);
    printf("Thread here, signing off. Good bye.\n");
}

int main() {
    int sock;                    /* fd for main socket */
    int msgsock;                 /* fd from accept return */
    struct sockaddr_in server;   /* socket struct for server connection */
    struct sockaddr_in client;   /* socket struct for client connection */
    int clientLen;               /* returned length of client from accept() */
    int rval;                    /* return value from read() */

    // allocate char* and int for getting quote
    last_line_char = 0;
    line_from_file = malloc(sizeof(char) * MAX_LINE_LEN);

    // setup randomizer from system time
    srand(time(NULL));

    // initialize log file pointer
    log_file = fopen(LOG_FILE_LOC, "w");

    // fill arrays with config data
    num_cats = init_cats_from_config(MAX_QUOTE_FILES, cat_names,
                                         cat_file_locs);

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
    while (1) {
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