#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "tokenizer.h"

/*
 * Ben Ellerby, CSCI 352, Summer 2016
 * This function tokenizes a provided cstring according to the provided delimiters.
 * Returns an array of cstrings, each element a token.
 */
char **tokenize(char *line, char *delimiters) {
    char* current_token;
    int token_count = 0;
    char* copy1 = malloc(strlen(line));
    char* copy2 = malloc(strlen(line));
    memcpy(copy1, line, strlen(line));
    memcpy(copy2, line, strlen(line));
    current_token = strtok(copy1, delimiters);
    // Traverse the string, count the tokens
    while (current_token != NULL) {
        token_count++;
        current_token = strtok(NULL, delimiters);
    }
    // Allocate appropriate array size
    char** tokens = malloc(sizeof(char*) * (token_count + 1));

    int index = 0;
    current_token = strtok(copy2, delimiters);
    // Traverse the string again, copy tokens
    while (current_token != NULL) {
        current_token[strcspn(current_token, "\n")] = 0;
        tokens[index] = current_token;
        index++;
        current_token = strtok(NULL, delimiters);
    }
    tokens[index] = '\0';
    return tokens;
}

/*
 * Returns the number of tokens in a provided array of tokens.
 */
int token_count(char **tokens) {
    int count = 0;
    int i;
    for (i = 0; tokens[i]; i++) {
        count++;
    }
    return count;
}
