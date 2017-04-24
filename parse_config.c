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

res* parse_config(char* path) {
    int status;
    FILE* conf = fopen(path, "r");
    // can read file
    res* head = NULL;
    res* tail = NULL;
    if (conf) {
        // do parsing
        char* line = NULL;
        size_t len = 0;
        ssize_t read;
        // iterate over input lines
        char** columns = NULL;
        char** elements = NULL;
        char** range = NULL;
        while ((read = getline(&line, &len, conf)) != -1) {
            // columns[0] = port, columns[1] = uid, columns[2] = gid
            columns = tokenize(line, ":");
            int i,j;
            range_node* port_head = NULL;
            range_node* uid_head = NULL;
            range_node* gid_head = NULL;
            range_node* port_tail = NULL;
            range_node* uid_tail = NULL;
            range_node* gid_tail = NULL;
            /*
            port = malloc(sizeof(range_node));
            uid = malloc(sizeof(range_node));
            gid = malloc(sizeof(range_node));
            */
            // iterate over colon-delineated segments
            for (i = 0; columns[i]; i++) {
                elements = tokenize(columns[i], ",");
				/*
                if (i == 0 && token_count(elements) != 1) {
                    printf("can't have multiple ports or ranges");
                }
				*/
                // iterate over comma-delineated segments
                for (j = 0; elements[j]; j++) {
                    range = tokenize(elements[j], "-");
                    range_node new_range;
                    if (token_count(range) > 2) {
                        printf("ranges must have 0 or 1 dashes");
                    }
                    else if (token_count(range) == 2) {
                        range_node n = {NULL, {atoi(range[0]), atoi(range[1])}};
                        new_range = n;
                    }
                    else {
                        range_node n = {NULL, {atoi(range[0]), 0}};
                        new_range = n;
                    }
                    if (i == 0) { // port
                        if (port_head == NULL) {
                            port_head = malloc(sizeof(range_node));
                            *port_head = new_range;
                            port_tail = port_head;
                        }
                        else {
                            port_tail->next = malloc(sizeof(range_node));
                            *(port_tail->next) = new_range;
                            port_tail = port_tail->next;
                        }
                    }
                    else if (i == 1) { // uid
                        if (uid_head == NULL) {
                            uid_head = malloc(sizeof(range_node));
                            *uid_head = new_range;
                            uid_tail = uid_head;
                        }
                        else {
                            uid_tail->next = malloc(sizeof(range_node));
                            *(uid_tail->next) = new_range;
                            uid_tail = uid_tail->next;
                        }
                    }
                    else { // gid
                        if (gid_head == NULL) {
                            gid_head = malloc(sizeof(range_node));
                            *gid_head = new_range;
                            gid_tail = gid_head;
                        }
                        else {
                            gid_tail->next = malloc(sizeof(range_node));
                            *(gid_tail->next) = new_range;
                            gid_tail = gid_tail->next;
                        }
                    }
                }
            }
            //res new_res = {&port, &uid, &gid};
            if (head == NULL) {
                head = malloc(sizeof(res));
                head->port_head = port_head;
                head->uid_head = uid_head;
                head->gid_head = gid_head;
                head->next = NULL;
                tail = head;
            }
            else {
                tail->next = malloc(sizeof(res));
                tail->next->port_head = port_head;
                tail->next->uid_head = uid_head;
                tail->next->gid_head = gid_head;
                tail->next->next = NULL;
                tail = tail->next;
            }
        }
    }
    // can't read file
    else {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    return head;
}

/*
int main() {
    res* r = parse_config("sprd.conf");
    while (r != NULL) {
        range_node* p = r->port_head;
        range_node* u = r->uid_head;
        range_node* g = r->gid_head;
        printf("Unpacking reservation...\n");
        while (p != NULL) {
            printf("Port range: %d %d\n", p->range[0], p->range[1]);
            p = p->next;
        }
        while (u != NULL) {
            printf("uid range: %d %d\n", u->range[0], u->range[1]);
            u = u->next;
        }
        while (g != NULL) {
            printf("gid range: %d %d\n", g->range[0], g->range[1]);
            g = g->next;
        }
        printf("End of reservation\n");
        r = r->next;
    }
    return 0;
}*/
