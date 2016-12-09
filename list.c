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

void push_back(List* list, unsigned int key) {
    if(is_empty(list)) {
        list->head = (Node*)malloc(sizeof(Node));
        if(list->head == NULL) {
            puts("Failed to allocate memory");
            exit(1);
        }
        init_node(list->head);
        list->last = list->head;
        list->head->key = key;
        return;
    } else {
        list->last->next_node = (Node*)malloc(sizeof(Node));
        if(list->last->next_node == NULL) {
            puts("Failed to allocate memory");
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

void push_front(List* list, unsigned int key) {
    if(is_empty(list)) {
        list->head = (Node*)malloc(sizeof(Node));
        if(list->head == NULL) {
            puts("Failed to allocate memory");
            exit(1);
        }
        init_node(list->head);
        list->last = list->head;
        list->head->key = key;
        return;
    } else {
        list->head->prev_node = (Node*)malloc(sizeof(Node));
        if(list->head->prev_node == NULL) {
            puts("Failed to allocate memory");
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

unsigned int securely_get_value(Node* current_node) {
    if(current_node == NULL)
        return 0;
    else
        return current_node->key;
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
