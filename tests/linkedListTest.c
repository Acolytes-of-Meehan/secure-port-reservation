
#include "linked_list.h"
#include <stdlib.h>
#include <stdio.h>

int main() {

  typedef struct tester {

    int data;

  } testNode;

  testNode a;
  a.data = 43;
  testNode b;
  b.data = 0;
  testNode c;
  c.data = 8;
  
  list_node *linked_list = make_linked_list(&a);

  add_to_list(&b, linked_list);

  add_to_list(&c, linked_list);

  remove_from_list(1, linked_list->next, linked_list);

  testNode *d = (testNode *)linked_list->listItem;
  testNode *e = (testNode *)linked_list->next->listItem;

  printf("%d\n", d->data);
  printf("%d\n", e->data);
  
  return 0;
}
