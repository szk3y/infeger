#pragma once

#include "list.h"

// char*型の要素に直接代入をしてはいけない
// 必ずupdate関数を使うこと
typedef struct {
    int is_negative;
    List unsigned_value;
    char* decimal_string;
    char* binary_string;
    char* hex_string;
} LargeInt;

void init_large_int(LargeInt*); // LargeIntは最初に必ずこの関数で初期化する
void hex_string_to_large_int(char*, LargeInt*);
static unsigned int word_to_uint(char*, int, int);
static unsigned int hex_char_to_uint(char);
void large_add(LargeInt*, LargeInt*, LargeInt*); // large_add(a,b,c) -> c = a + b
void large_sub(LargeInt*, LargeInt*, LargeInt*); // large_sub(a,b,c) -> c = a - b
void update_hex_string(LargeInt*);
void update_binary_string(LargeInt*);
static void swap(char*, char*);
static void reverse_string(char*);
void release_large_int(LargeInt*); // LargeIntは最後に必ずこの関数でメモリを開放する
void print_number(LargeInt*);
int get_digit(LargeInt*);
void print_hex(LargeInt*);
void print_binary(LargeInt*);
