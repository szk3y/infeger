#pragma once

#include <stdint.h>

// HACK: struct nodeなしにしたい
typedef struct node Node;
typedef struct node {
    uint32_t key;
    Node* next_node;
    Node* prev_node;
} Node;

typedef struct {
    Node* head;
    Node* last;
} List;

void init_node(Node*); // Nodeは最初に必ずこの関数を使って初期化する
void init_list(List*); // Listは最初に必ずこの関数を使って初期化する
void copy_list(List* origin, List* clone); // clone = origin
int is_empty(List*);
int get_length(List*); // ノードの数を返す
void push_back(List*, uint32_t); // Listの後ろに新しいノードをつける
void push_front(List*, uint32_t); // Listの前に新しいノードをつける
void pop_front(List*); // Listの先頭要素を破棄する
Node* securely_get_prev_node(Node*); // NULLを渡されると停止せずNULLを返す
uint32_t securely_get_value(Node*); // NULLを渡されると停止せず0を返す
int has_prev_prev_node(Node*); // node->prev->prev == NULLを返す．ぬるぽ防止
int has_next_next_node(Node*); // node->next->next == NULLを返す．ぬるぽ防止
void release_list(List*); // Listは最後に必ずこの関数を使ってメモリを開放する
void debug_print_list(List*); // Listの情報を表示する
void debug_print_node(Node*); // Nodeの情報を表示する
void print_address(void*); // 0xaaもしくはNULLのようにアドレスを表示して改行
