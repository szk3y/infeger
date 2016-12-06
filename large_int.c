#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "large_int.h"

#define DEBUG

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

}

void hex_string_to_large_int(char* hex_string, LargeInt* large_int) {
    large_int->is_negative = (hex_string[0] == '-');

    int length = kHexDigitsInUInt;
    // 文字列が負のとき，iは1から始まる．
    for(int i = large_int->is_negative; i < (int)strlen(hex_string); i = i + length) {
        if(i == large_int->is_negative && strlen(hex_string) % kHexDigitsInUInt != 0) {
            length = strlen(hex_string) % kHexDigitsInUInt;
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

// result = former + latter
// 符号は気にせず加算を行う
static void large_add(LargeInt* former, LargeInt* latter, LargeInt* result) {
    LargeInt buffer;
    init_large_int(&buffer);
    unsigned long carry = 0;
    Node* iter1 = former->unsigned_value.last;
    Node* iter2 = latter->unsigned_value.last;
    while(iter1 != NULL || iter2 != NULL || carry != 0) {
        unsigned long new_value =
            (unsigned long)securely_get_value(iter1) +
            (unsigned long)securely_get_value(iter2) + carry;
        // new_valueの下半分を取り出して代入
        push_front(&buffer->unsigned_value, (unsigned int)new_value);
        // new_valueの上半分を桁上げとして保存
        carry = new_value >> (sizeof(unsigned int) * 8);
        iter1 = securely_get_prev_node(iter1);
        iter2 = securely_get_prev_node(iter2);
    }
    release_large_int(&result);
}

// former - latterを行う
// |former| < |latter|のときはlarge_sub(latter, former, result)する
static void large_sub(LargeInt* former, LargeInt* latter, LargeInt* result) {
    release_large_int(result);
    if(is_less_than(former, latter)) {
        large_sub(latter, former, result);
        return;
    }

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

    if(large_int->decimal_string != NULL)
        free(large_int->decimal_string);
    if(large_int->binary_string != NULL)
        free(large_int->binary_string);
    if(large_int->hex_string != NULL)
        free(large_int->hex_string);
}

void print_hex(LargeInt* large_int) {
    puts(large_int->hex_string);
}

void print_binary(LargeInt* large_int) {
    puts(large_int->binary_string);
}

#ifdef DEBUG
int main() {
    char buffer[256];
    LargeInt large_int_1;
    init_large_int(&large_int_1);
    scanf("%s", buffer);
    hex_string_to_large_int(buffer, &large_int_1);
    update_hex_string(&large_int_1);
    printf("large_int_1 = 0x");
    print_hex(&large_int_1);

    LargeInt large_int_2;
    init_large_int(&large_int_2);
    scanf("%s", buffer);
    hex_string_to_large_int(buffer, &large_int_2);
    update_hex_string(&large_int_2);
    printf("large_int_2 = 0x");
    print_hex(&large_int_2);

    LargeInt result;
    init_large_int(&result);
    large_add(&large_int_1, &large_int_2, &result);

    update_hex_string(&result);
    printf("result = 0x");
    print_hex(&result);
    update_binary_string(&result);
    printf("result = 0b");
    print_binary(&result);

    release_large_int(&large_int_1);
    release_large_int(&large_int_2);
    release_large_int(&result);
    return 0;
}
#endif // DEBUG
