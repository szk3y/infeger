#pragma once

// HACK: struct nodeなしにしたい
typedef struct node Node;
typedef struct node {
    unsigned int key;
    Node* next_node;
    Node* prev_node;
} Node;

typedef struct {
    Node* head;
    Node* last;
} List;

void init_node(Node*); // Nodeは最初に必ずこの関数を使って初期化する
void init_list(List*); // Listは最初に必ずこの関数を使って初期化する
void copy_list(List*, List*); // copy_list(a,b) => b = a
int is_empty(List*);
int get_length(List*); // ノードの数を返す
void push_back(List*, unsigned int); // Listの後ろに新しいノードをつける
void push_front(List*, unsigned int); // Listの前に新しいノードをつける
Node* securely_get_prev_node(Node*); // NULLを渡されると停止せずNULLを返す
unsigned int securely_get_value(Node*); // NULLを渡されると停止せず0を返す
void release_list(List*); // Listは最後に必ずこの関数を使ってメモリを開放する
