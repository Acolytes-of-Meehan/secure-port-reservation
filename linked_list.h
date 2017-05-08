/* linked_list.h
 *
 * Ben Ellerby
 * Ray Weiming Luo
 * Evan Ricks
 * Oliver Smith-Denny
 *
 */

#include <stdlib.h>

typedef struct linkedListNode {

  void *listItem;
  struct linkedListNode *next;

} list_node;

list_node * make_linked_list();
int add_to_list(void *listItem, list_node *list);
int remove_from_list(int position, list_node *deleteNode, list_node *list);


