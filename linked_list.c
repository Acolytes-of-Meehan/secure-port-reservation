/* 
 * Linked List Implementation
 * Benjamin Ellerby
 * Ray Luow
 * Evan Ricks
 * Oliver Smith-Denny
 *
 */

#include "linked_list.h"
#define RETURN_FAILURE -1
#define RETURN_SUCCESS 0

list_node * make_linked_list () {

  list_node *new_list = calloc(1, sizeof(list_node));

  new_list->listItem = NULL;
  new_list->next = NULL;

  return new_list;

}

int add_to_list (void *listItem, list_node *list) {

  if (listItem == NULL) {
    return RETURN_FAILURE;
  }

  list_node *curr = list;

  if (curr->listItem == NULL) {
    curr->listItem = listItem;
  } else {

    while (curr->next != NULL)
      curr = curr->next;

    list_node *new = calloc(1, sizeof(list_node));
    new->next = NULL;
    new->listItem = listItem;

    curr->next = new;
  }

  return RETURN_SUCCESS;

}

int remove_from_list (int position, list_node *deleteNode, list_node *list) {

  if (list == NULL || position < 0 || deleteNode == NULL) {
    return RETURN_FAILURE;
  }

  list_node *curr = list;
  
  for(int i = 0; i < position - 1; i++) {
    if (curr->next != NULL) {
      curr = curr->next;
    } else {
      return RETURN_FAILURE;
    }
  }

  if (curr->next == NULL) {
    return RETURN_FAILURE;
  }

  if (curr->next->next == NULL) {
    curr->next = NULL;
  } else {
    curr->next = curr->next->next;
  }

  free(deleteNode);

  return RETURN_SUCCESS;
}
