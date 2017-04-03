#include <stdio.h>
#include <stdlib.h>
#include "parse_config.h"
#include "tokenizer.h"

int is_in_range(range_node* node, int val) {
    if (node == NULL) {
        return 0;
    }
    if ((node->range[0] == val) || ((val > node->range[0]) && (val <= node->range[1]))) {
        return 1;
    }
    else {
        return is_in_range(node->next, val);
    }
}

int parse_config(char* path, res* reservations) {
    int status;
    FILE* conf = fopen(path, "r");
    // can read file
    if (conf) {
        // do parsing
        char* line = NULL;
        size_t len = 0;
        ssize_t read;
        // iterate over input lines
        char** tokens = NULL;
        while ((read = getline(&line, &len, conf)) != -1) {
            // tokens[0] = port, tokens[1] = uid, tokens[2] = gid
            tokens = tokenize(line, ":");
        }
    }
    // can't read file
    else {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    return status;
}

int main() {
    return 0;
}
