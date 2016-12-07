#include <stdio.h>
#include "list.h"

int main() {
    List list;
    init_list(&list);
    printf("is_empty: %d\n", is_empty(&list));
    push_back(&list, 99);
    printf("%u\n", list.head->key);
    release_list(&list);
    push_back(&list, 100);
    push_back(&list, 101);
    push_front(&list, 102);
    for(Node* iter = list.head; iter != NULL; iter = iter->next_node) {
        printf("%u\n", iter->key);
    }
    printf("prev_node->key: %u\n", list.last->prev_node->key);
    release_list(&list);
    return 0;
}
