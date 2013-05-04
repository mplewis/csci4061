#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <pthread.h>

#define LOG_FILE_LOC          "server.log"
#define QUOTE_CONFIG_FILE_LOC "config.txt"
#define MAX_QUOTE_FILES       16
#define MAX_LINE_LEN          1024
#define MAX_NAME_FILENAME_LEN 256

FILE *fp;
FILE *log_file;
char *line_from_file;
size_t max_line_len = MAX_LINE_LEN;
ssize_t num_chars_in_line;
FILE *file_pointers[MAX_QUOTE_FILES] = {0};
pthread_mutex_t *log_lock;

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
    while (num_chars_in_line = getline(&line_from_file, &max_line_len, fp)
           != -1) {
        if (num_categories == max_num_cats) {
            break;
        }
        // strip newline char from line
        int last_line_char = strlen(line_from_file) - 1;
        if (line_from_file[last_line_char] == '\n') {
            line_from_file[last_line_char] = '\0';
        }
        token = NULL;
        token = strtok(line_from_file, ": ");
        int token_count = 0;
        while (token != NULL) {
            // copy and store tokens in the proper array
            char *token_copy = malloc(sizeof(char) * MAX_NAME_FILENAME_LEN);
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
            printf("Failure opening %s\n", QUOTE_CONFIG_FILE_LOC);
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

    while (num_chars_in_line = getline(&line_from_file, &max_line_len,
           cat_file_pointer) == -1) {
        rewind(cat_file_pointer);
    }
    // strip newline char from line
    last_line_char = strlen(line_from_file) - 1;
    if (line_from_file[last_line_char] == '\n') {
        line_from_file[last_line_char] = '\0';
    }
    strcpy(quote_text, line_from_file);

    while (num_chars_in_line = getline(&line_from_file, &max_line_len,
           cat_file_pointer) == -1) {
        rewind(cat_file_pointer);
    }
    // strip newline char from line
    last_line_char = strlen(line_from_file) - 1;
    if (line_from_file[last_line_char] == '\n') {
        line_from_file[last_line_char] = '\0';
    }
    strcpy(quote_author, line_from_file);
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

int main() {
    // setup randomizer from system time
    srand(time(NULL));

    // initialize log file pointer and logging mutex
    pthread_mutex_init(log_lock, NULL);
    log_file = fopen(LOG_FILE_LOC, "w");

    // initialize category arrays
    char *cat_names[MAX_QUOTE_FILES] = {0};
    char *cat_file_locs[MAX_QUOTE_FILES] = {0};

    // fill arrays with config data
    int num_cats = init_cats_from_config(MAX_QUOTE_FILES, cat_names,
                                         cat_file_locs);
    
    // debug config.txt info to stdout
    printf("Number of categories: %i\n", num_cats);
    for (int i = 0; i < num_cats; i++) {
        printf("    - %s (%s)\n", cat_names[i], cat_file_locs[i]);
    }

    // setup quote text and author strings
    char *text = malloc(sizeof(char) * MAX_LINE_LEN);
    char *author = malloc(sizeof(char) * MAX_LINE_LEN);
    open_fps_from_cat_file_locs(file_pointers, num_cats, cat_file_locs);

    // print 8 quotes from randomly-selected categories
    //*
    for (int i = 0; i < 8; i++) {
        get_quote_from_specific_category(text, author, num_cats, cat_names,
                                         file_pointers, "Any");
        printf("%s by %s\n", text, author);
    }
    //*/

    // write some sample log items
    for (int i = 0; i < 10; i++) {    
        pthread_mutex_lock(log_lock);
        fprintf(log_file, "%i %s\n", i, "<-- There's a test log item!");
        pthread_mutex_unlock(log_lock);
    }

    // close log file
    if (fclose(log_file) != 0) {
        printf("Error closing log file %s\n", LOG_FILE_LOC);
    }

    // close all open file pointers
    close_all_fps(num_cats, file_pointers);

    return 0;
}