#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "large_int.h"

static unsigned int word_to_uint(char*, int, int);
static unsigned int hex_char_to_uint(char);
static void large_add(LargeInt*, LargeInt*, LargeInt*);
static void large_sub(LargeInt*, LargeInt*, LargeInt*);
static int is_less_than(LargeInt*, LargeInt*);
static void swap(char*, char*);
static void reverse_string(char*);

// unsigned intの16進数での桁数
static const int kHexDigitsInUInt = sizeof(unsigned int) * 2;

// LargeIntは最初に必ずこの関数を使って初期化する
void init_large_int(LargeInt* large_int) {
    large_int->is_negative = 0;
    init_list(&large_int->unsigned_value);
    large_int->decimal_string = NULL;
    large_int->binary_string  = NULL;
    large_int->hex_string     = NULL;
}

void copy_large_int(LargeInt* origin, LargeInt* clone) {
    release_large_int(clone);
    copy_list(&origin->unsigned_value, &clone->unsigned_value);
}

void hex_string_to_large_int(char* hex_string, LargeInt* large_int) {
    release_large_int(large_int);
    large_int->is_negative = (hex_string[0] == '-');

    int length = kHexDigitsInUInt;
    // 文字列が負のとき，iは1から始まる．
    for(int i = large_int->is_negative; i < (int)strlen(hex_string); i = i + length) {
        // 最初の処理は残りの桁数がunsigned intで割り切れるように調節する
        if(i == large_int->is_negative && (strlen(hex_string) - large_int->is_negative) % kHexDigitsInUInt != 0) {
            length = (strlen(hex_string) - large_int->is_negative) % kHexDigitsInUInt;
        } else {
            length = kHexDigitsInUInt;
        }
        push_back(&large_int->unsigned_value, word_to_uint(hex_string, i, length));
    }
}

// unsigned intと同じ大きさの文字列で表された数字をunsigned intにして返す
static unsigned int word_to_uint(char* hex_string, int beginning_index, int length) {
    unsigned int result = 0;
    for(int i = 0; i < length; i++) {
        if(hex_string[beginning_index + i] == '\0')
            break;
        result += hex_char_to_uint(hex_string[beginning_index + i]) << (4 * (length - i - 1));
    }
    return result;
}

static unsigned int hex_char_to_uint(char hex_char) {
    switch(hex_char) {
        case '0':
            return 0;
        case '1':
            return 1;
        case '2':
            return 2;
        case '3':
            return 3;
        case '4':
            return 4;
        case '5':
            return 5;
        case '6':
            return 6;
        case '7':
            return 7;
        case '8':
            return 8;
        case '9':
            return 9;
        case 'a':
        case 'A':
            return 10;
        case 'b':
        case 'B':
            return 11;
        case 'c':
        case 'C':
            return 12;
        case 'd':
        case 'D':
            return 13;
        case 'e':
        case 'E':
            return 14;
        case 'f':
        case 'F':
            return 15;
        default:
            puts("hex_char_to_uint: Invalid input");
            exit(1);
    }
}

// 符号に応じて加算か減算を行う
void large_plus(LargeInt* former, LargeInt* latter, LargeInt* result) {
    // 同符号の場合
    if(former->is_negative == latter->is_negative) {
        large_add(former, latter, result);
        result->is_negative = former->is_negative;
    // 異符号の場合
    } else {
        // |former| < |latter|のときはlarge_sub(latter, former, result)する
        if(is_less_than(former, latter)) {
            large_sub(latter, former, result);
            result->is_negative = latter->is_negative;
        } else {
            large_sub(former, latter, result);
            result->is_negative = former->is_negative;
        }
    }
}

// latterの符号を変えてlarge_plusを行う
void large_minus(LargeInt* former, LargeInt* latter, LargeInt* result) {
    latter->is_negative = !latter->is_negative;
    large_plus(former, latter, result);
}

// result = former + latter
// 符号は気にせず加算を行う
static void large_add(LargeInt* former, LargeInt* latter, LargeInt* result) {
    // large_add(a, b, a)などにも対応するためresultは最後に触る
    LargeInt buffer;
    init_large_int(&buffer);
    unsigned long carry = 0;
    Node* former_node = former->unsigned_value.last;
    Node* latter_node = latter->unsigned_value.last;
    while(former_node != NULL || latter_node != NULL || carry != 0) {
        // 片方が短くてもぬるぽしないようにsecurely~を使う
        unsigned long new_value =
            (unsigned long)securely_get_value(former_node) +
            (unsigned long)securely_get_value(latter_node) + carry;
        // new_valueの下半分を取り出して代入
        push_front(&buffer.unsigned_value, (unsigned int)new_value);
        // new_valueの上半分を桁上げとして保存
        carry = new_value >> (sizeof(unsigned int) * 8);
        // 片方が短くてもぬるぽしないようにsecurely~を使う
        former_node = securely_get_prev_node(former_node);
        latter_node = securely_get_prev_node(latter_node);
    }
    copy_large_int(&buffer, result);
    release_large_int(&buffer);
}

// former - latterを行う
// 符号は全く気にしない
static void large_sub(LargeInt* former, LargeInt* latter, LargeInt* result) {
    // large_sub(a, b, a)などにも対応するためresultは最後に触る
    LargeInt buffer;
    init_large_int(&buffer);
    Node* former_node = former->unsigned_value.last;
    Node* latter_node = latter->unsigned_value.last;
    unsigned long carry = 0;
    // carry が残ることはない
    while(former_node != NULL || latter_node != NULL) {
        // 片方のリストが短くてもぬるぽしないようにsecurely~を使う
        // 繰り下がりのぶんを予め足しておく
        unsigned long new_value =
            ((unsigned long)1 << (kHexDigitsInUInt * 4)) +
            (unsigned long)securely_get_value(former_node) -
            (unsigned long)securely_get_value(latter_node) - carry;
        // new_valueの下半分を取り出して代入
        push_front(&buffer.unsigned_value, (unsigned int)new_value);
        // new_valueの上半分をみて繰り下がり判定
        carry = (new_value >> (kHexDigitsInUInt * 4)) == 0;
        // 片方のリストが短くてもぬるぽしないようにsecurely~を使う
        former_node = securely_get_prev_node(former_node);
        latter_node = securely_get_prev_node(latter_node);
    }
    copy_large_int(&buffer, result);
    release_large_int(&buffer);
}

// |former| < |latter|を返す
static int is_less_than(LargeInt* former, LargeInt* latter) {
    if(get_length(&former->unsigned_value) != get_length(&latter->unsigned_value))
        return get_length(&former->unsigned_value) < get_length(&latter->unsigned_value);
    Node* former_node = former->unsigned_value.head;
    Node* latter_node = latter->unsigned_value.head;
    // 値が等しいときは次のノードにいく
    while(former_node != NULL && former_node->key == latter_node->key) {
        former_node = former_node->next_node;
        latter_node = latter_node->next_node;
    }
    return securely_get_value(former_node) < securely_get_value(latter_node);
}

// LargeIntのunsigned_valueからhex_stringを更新する
void update_hex_string(LargeInt* large_int) {
    if(large_int->hex_string != NULL)
        free(large_int->hex_string);
    int string_length = get_length(&large_int->unsigned_value) * kHexDigitsInUInt;

    if(string_length == 0) {
        large_int->hex_string = (char*)malloc(sizeof(char) * 2);
        if(large_int->hex_string == NULL) {
            puts("Failed to allocate memory");
            exit(1);
        }
        large_int->hex_string[0] = '0';
        large_int->hex_string[1] = '\0';
    }

    large_int->hex_string = (char*)malloc(sizeof(char) * (string_length + 1));
    if(large_int->hex_string == NULL) {
        puts("Failed to allocate memory");
        exit(1);
    }
    Node* current_node = large_int->unsigned_value.head;
    for(int i = 0; i < get_length(&large_int->unsigned_value); i++) {
        snprintf(large_int->hex_string + i * kHexDigitsInUInt, kHexDigitsInUInt + 1, "%08x", current_node->key);
        current_node = current_node->next_node;
    }
}

// LargeIntのunsigned_valueからbinary_stringを更新する
void update_binary_string(LargeInt* large_int) {
    if(large_int->binary_string != NULL)
        free(large_int->binary_string);
    int string_length = get_length(&large_int->unsigned_value) * kHexDigitsInUInt * 4;

    if(string_length == 0) {
        large_int->binary_string = (char*)malloc(sizeof(char) * 2);
        if(large_int->binary_string == NULL) {
            puts("Failed to allocate memory");
            exit(1);
        }
        large_int->binary_string[0] = '0';
        large_int->binary_string[1] = '\0';
    }

    large_int->binary_string = (char*)malloc(sizeof(char) * (string_length + 1));
    if(large_int->binary_string == NULL) {
        fprintf(stderr, "Failed to allocate memory\n");
        exit(1);
    }
    Node* current_node = large_int->unsigned_value.last;
    for(int i = 0; current_node != NULL; current_node = current_node->prev_node) {
        unsigned int current_value = current_node->key;
        for(int j = 0; j < kHexDigitsInUInt * 4; j++) {
            if(current_value % 2 == 0) {
                large_int->binary_string[i] = '0';
            } else {
                large_int->binary_string[i] = '1';
            }
            current_value = current_value >> 1;
            i++;

            // 大量の0が頭につくのを防ぐ
            if(current_value == 0 && current_node == large_int->unsigned_value.head)
                break;
        }
    }
    reverse_string(large_int->binary_string);
}

static void swap(char* a, char* b) {
    char tmp = *a;
    *a = *b;
    *b = tmp;
}

static void reverse_string(char* string) {
    int length = strlen(string);
    for(int i = 0; i < length / 2; i++) {
        swap(&string[i], &string[length - i - 1]);
    }
}

// LargeIntは最後に必ずこの関数を使ってメモリを開放する
void release_large_int(LargeInt* large_int) {
    release_list(&large_int->unsigned_value);

    if(large_int->decimal_string != NULL) {
        free(large_int->decimal_string);
        large_int->decimal_string = NULL;
    }
    if(large_int->binary_string != NULL) {
        free(large_int->binary_string);
        large_int->binary_string = NULL;
    }
    if(large_int->hex_string != NULL) {
        free(large_int->hex_string);
        large_int->hex_string = NULL;
    }
}

void print_hex(LargeInt* large_int) {
    puts(large_int->hex_string);
}

void print_binary(LargeInt* large_int) {
    puts(large_int->binary_string);
}
