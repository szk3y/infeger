#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "list.h"


void init_node(Node* node) {
    node->key = 0;
    node->next_node = NULL;
    node->prev_node = NULL;
}

void init_list(List* list) {
    list->head = NULL;
    list->last = NULL;
}

void copy_list(List* origin, List* clone) {
    release_list(clone);
    for(Node* node = origin->head; node != NULL; node = node->next_node) {
        push_back(clone, node->key);
    }
}

int is_empty(List* list) {
    return list->head == NULL;
}

int get_length(List* list) {
    int counter = 0;
    Node* current_node = list->head;
    while(current_node != NULL) {
        counter++;
        current_node = current_node->next_node;
    }
    return counter;
}

void push_back(List* list, uint32_t key) {
    if(is_empty(list)) {
        list->head = (Node*)malloc(sizeof(Node));
        if(list->head == NULL) {
            fprintf(stderr, "Failed to allocate memory\n");
            exit(1);
        }
        init_node(list->head);
        list->last = list->head;
        list->head->key = key;
    } else {
        list->last->next_node = (Node*)malloc(sizeof(Node));
        if(list->last->next_node == NULL) {
            fprintf(stderr, "Failed to allocate memory\n");
            exit(1);
        }
        init_node(list->last->next_node);
        // 新しいノードをリストの最後のノードとつなぐ
        list->last->next_node->prev_node = list->last;
        // リストの最後のノードを更新する
        list->last = list->last->next_node;

        list->last->key = key;
    }
}

void push_front(List* list, uint32_t key) {
    if(is_empty(list)) {
        list->head = (Node*)malloc(sizeof(Node));
        if(list->head == NULL) {
            fprintf(stderr, "Failed to allocate memory\n");
            exit(1);
        }
        init_node(list->head);
        list->last = list->head;
        list->head->key = key;
    } else {
        list->head->prev_node = (Node*)malloc(sizeof(Node));
        if(list->head->prev_node == NULL) {
            fprintf(stderr, "Failed to allocate memory\n");
            exit(1);
        }
        init_node(list->head->prev_node);
        // 新しいノードをリストの先頭のノードとつなぐ
        list->head->prev_node->next_node = list->head;
        // リストの先頭のノードを更新する
        list->head = list->head->prev_node;

        list->head->key = key;
    }
}

void pop_front(List* list) {
    if(is_empty(list))
        return;
    // 消すノードのポインタを保存してリストの先頭ノードを更新する
    Node* target = list->head;
    list->head = list->head->next_node;

    // 消す要素と新しい先頭ノードを切り離す
    list->head->prev_node = NULL;

    free(target);
}

Node* securely_get_prev_node(Node* current_node) {
    if(current_node == NULL)
        return NULL;
    else
        return current_node->prev_node;
}

Node* securely_get_next_node(Node* current_node) {
    if(current_node == NULL)
        return NULL;
    else
        return current_node->next_node;
}

uint32_t securely_get_value(Node* current_node) {
    if(current_node == NULL)
        return 0;
    else
        return current_node->key;
}

int has_prev_prev_node(Node* node) {
    return securely_get_prev_node(securely_get_prev_node(node)) == NULL;
}

int has_next_next_node(Node* node) {
    return securely_get_next_node(securely_get_next_node(node)) == NULL;
}

void release_list(List* list) {
    if(is_empty(list))
        return;
    // 先頭以外のすべてのノードを開放する
    for(Node* iter = list->last->prev_node; iter != NULL; iter = iter->prev_node) {
        free(iter->next_node);
    }
    free(list->head);
    list->head = NULL;
    list->last = NULL;
}

void debug_print_list(List* list) {
    printf("  List:           %p\n", list);
    printf("    head:  ");
    print_address(list->head);
    printf("    last:  ");
    print_address(list->last);
    printf("    value: 0x");
    for(Node* node = list->head; node != NULL; node = node->next_node) {
        printf(" %08x", node->key);
    }
    puts("");
    printf("    -----head-----\n");
    for(Node* node = list->head; node != NULL; node = node->next_node) {
        debug_print_node(node);
    }
    printf("    -----last-----\n");
}

void debug_print_node(Node* node) {
    printf("      Node: %p\n", node);
    printf("        prev_node: ");
    print_address(node->prev_node);
    printf("        key:       0x%08x\n", node->key);
    printf("        next_node: ");
    print_address(node->next_node);
}

void print_address(void* pointer) {
    if(pointer == NULL) {
        printf("NULL\n");
    } else {
        printf("%p\n", pointer);
    }
}
