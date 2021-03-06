#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "list.h"

// char*型の要素に直接代入をしてはいけない
typedef struct {
    bool is_negative;
    List unsigned_value;
    char* decimal_string;
    char* binary_string;
    char* hex_string;
} LargeInt;

void init_large_int(LargeInt*); // LargeIntは最初に必ずこの関数で初期化する
void copy_large_int(LargeInt* origin, LargeInt* clone); // clone = origin; 順番逆にしたい
void hex_string_to_large_int(char*, LargeInt*);
void decimal_string_to_large_int(char*, LargeInt*);
void large_plus(LargeInt* former, LargeInt* latter, LargeInt* result);     // result = former + latter
void large_minus(LargeInt* former, LargeInt* latter, LargeInt* result);    // result = former - latter
void large_multiply(LargeInt* former, LargeInt* latter, LargeInt* result); // result = former * latter
void large_divide(LargeInt* former, LargeInt* latter, LargeInt* result);   // result = former / latter
void release_large_int(LargeInt*); // LargeIntは最後に必ずこの関数でメモリを開放する
void print_number(LargeInt*);
int get_digit(LargeInt*);
void print_hex_string(LargeInt*);
void print_binary_string(LargeInt*);
void print_decimal_string(LargeInt*);
void debug_print_large_int(LargeInt*);
