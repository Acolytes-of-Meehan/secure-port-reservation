#include <stdio.h>
#include <stdlib.h>

typedef struct range_node {
    struct range_node* next;
    int range[2];
} range_node;

typedef struct res {
    range_node* port_head;
    range_node* uid_head;
    range_node* gid_head;
    struct res* next;
} res;

int is_in_range(range_node* node, int val);

res* parse_config(char* path);
