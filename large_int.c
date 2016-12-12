#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "large_int.h"

static uint32_t word_to_uint(char*, int beginning_index, int length); // 16進数文字列の32bit整数を32bit整数にする
static uint32_t hex_char_to_uint(char); // 16進数の文字を整数にして返す
// 符号の処理はこの2つの関数の後で行わなければならない
static void large_add(LargeInt* a, LargeInt* b, LargeInt* result); // 符号を気にせず result = a + b
static void large_sub(LargeInt* a, LargeInt* b, LargeInt* result); // 符号を気にせず result = a - b

static int is_less_than(LargeInt* former, LargeInt* latter); // |former| < |latter|を返す
static void swap(char*, char*); // aとbを入れ替える
static void reverse_string(char*); // 文字列を反転させる
static void remove_zero_nodes(LargeInt*); // 左側の不要な値0のノードをできるだけ消す
static void push_back_zero_nodes(LargeInt*, int); // LargeIntをuint32_t n個分左にシフトする
static void multiply_large_and_small(LargeInt* a, uint32_t b, LargeInt* result); // result = a * b
static void large_shift_left(LargeInt*); // LargeIntを1つだけ左論理シフトする
static void large_shift_right(LargeInt*); // LargeIntを1つだけ右論理シフトする


// uint32_tの16進数での桁数(8)
static const int kHexDigitsInUInt = sizeof(uint32_t) * 2;
// uint32_tのビット数(32)
static const int kNumOfBitsInUInt = sizeof(uint32_t) * 8;

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
        // 最初の処理は残りの桁数がuint32_tで割り切れるように調節する
        if(i == large_int->is_negative && (strlen(hex_string) - large_int->is_negative) % kHexDigitsInUInt != 0) {
            length = (strlen(hex_string) - large_int->is_negative) % kHexDigitsInUInt;
        } else {
            length = kHexDigitsInUInt;
        }
        push_back(&large_int->unsigned_value, word_to_uint(hex_string, i, length));
    }
}

// beginning_indexを呼び出す側でhex_stringに足しておくというのもアリか？
// uint32_tと同じ大きさの文字列で表された数字をuint32_tにして返す
static uint32_t word_to_uint(char* hex_string, int beginning_index, int length) {
    uint32_t result = 0;
    for(int i = 0; i < length; i++) {
        if(hex_string[beginning_index + i] == '\0')
            break;
        // 1文字ずつ数値に変えて足していく
        result += hex_char_to_uint(hex_string[beginning_index + i]) << (4 * (length - i - 1));
    }
    return result;
}

static uint32_t hex_char_to_uint(char hex_char) {
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

// former + latter
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

// former - latter
// latterの符号を変えてlarge_plusを行う
void large_minus(LargeInt* former, LargeInt* latter, LargeInt* result) {
    latter->is_negative = !latter->is_negative;
    large_plus(former, latter, result);
}

// bufferやtmpのような名前が2つ出そうになったので変数名にorigin,cloneを使ってみた
void large_multiply(LargeInt* former, LargeInt* latter, LargeInt* clone) {
    // releaseのタイミングがややこしくなりそうなので符号を先に決めておく
    int is_negative = former->is_negative != latter->is_negative;
    LargeInt origin;
    init_large_int(&origin);
    // HACK: uint何個分左シフトするかを数える．他の方法を考える
    int counter = 0;
    for(Node* node = latter->unsigned_value.head; node != NULL; node = node->next_node) {
        LargeInt tmp;
        init_large_int(&tmp);
        // formerとlatterの一部をかけて結果をtmpに保存する
        multiply_large_and_small(former, node->key, &tmp);
        push_back_zero_nodes(&tmp, counter);
        counter++;
        large_add(&origin, &tmp, &origin);
        release_large_int(&tmp);
    }
    copy_large_int(&origin, clone);
    release_large_int(&origin);
    clone->is_negative = is_negative;
}

// HACK: 筆算方式に変える
void large_divide(LargeInt* former, LargeInt* latter, LargeInt* result) {
    int is_negative = former->is_negative != latter->is_negative;
    // この数字から引いていく
    LargeInt current_divident;
    init_large_int(&current_divident);
    copy_large_int(former, &current_divisor)
    // この数字を少しずつ小さくして引いていく
    LargeInt current_divisor;
    init_large_int(latter, &current_divisor);
    copy_large_int(latter, &current_divisor);
    // 結果を一時的に保持する
    LargeInt quotient;
    init_large_int(&quotient);
    // 引き算できる場合どのbitをオンにするかを示す
    LargeInt current_bit;
    init_large_int(&current_bit);
    hex_string_to_large_int("1", &current_bit);

    while(is_less_than(&current_divisor, &current_divident)) {

    }

    while(!is_less_than(&))
}

static void large_shift_left(LargeInt* large_int) {
    uint64_t carry = 0;
    // すべてのキーに対し左シフト
    for(Node* node = large_int->unsigned_value.head; node != NULL; node = node->next_node) {
        uint64_t new_value = ((uint64_t)node->key << 1) + carry;
        node->key = (uint32_t)new_value;
        // はみ出た分を桁上げとして保存
        carry = new_value >> kNumOfBitsInUInt;
    }
    if(carry == 1) {
        push_front(large_int, 1);
    }
}

static void large_shift_right(LargeInt* large_int) {
    uint32_t carry_flag = 0;
    for(Node* node = large_int->unsigned_value.head; node != NULL; node = node->next_node) {
        // 値が変わってからではわからないので最初に桁下げがないか確認する
        uint32_t current_carry_flag = node->key & 1;
        // キャリーフラグは一つ上位のuintから降ってくるので32bitで最上位にくる
        node->key = (node->key >> 1) + (carry_flag << 31);
        carry_flag = current_carry_flag;
    }
}

static void multiply_large_and_small(LargeInt* large, uint32_t small, LargeInt* result) {
    LargeInt buffer;
    init_large_int(&buffer);
    uint64_t carry = 0;
    Node* arg_node = large->unsigned_value.last;
    while(arg_node != NULL || carry != 0) {
        uint64_t new_value = (uint64_t)securely_get_value(arg_node) * (uint64_t)small + carry;
        // new_valueの下半分を取り出して代入
        push_front(&buffer.unsigned_value, (uint32_t)new_value);
        // new_valueの上半分を桁上げとして保存
        carry = new_value >> kNumOfBitsInUInt;
        arg_node = arg_node->prev_node;
    }
    remove_zero_nodes(&buffer);
    copy_large_int(&buffer, result);
    release_large_int(&buffer);
}

// LargeIntの末尾に0のノードをnum_of_zero_nodesだけつける．
// push_back一回でuint32_t一つ分左シフトしたことになる
static void push_back_zero_nodes(LargeInt* large_int, int num_of_zero_nodes) {
    for(int i = 0; i < num_of_zero_nodes; i++) {
        push_back(&large_int->unsigned_value, 0);
    }
}

// 先頭から0でないノードが出るまでノードを取り除く
static void remove_zero_nodes(LargeInt* large_int) {
    while(large_int->unsigned_value.head != large_int->unsigned_value.last
            && large_int->unsigned_value.head->key == 0) {
        pop_front(&large_int->unsigned_value);
    }
}

// result = former + latter
// 符号は気にせず加算を行う
static void large_add(LargeInt* former, LargeInt* latter, LargeInt* result) {
    // large_add(a, b, a)などにも対応するためresultは最後に触る
    LargeInt buffer;
    init_large_int(&buffer);
    uint64_t carry = 0;
    Node* former_node = former->unsigned_value.last;
    Node* latter_node = latter->unsigned_value.last;
    while(former_node != NULL || latter_node != NULL || carry != 0) {
        // 片方が短くてもぬるぽしないようにsecurely~を使う
        uint64_t new_value =
            (uint64_t)securely_get_value(former_node) +
            (uint64_t)securely_get_value(latter_node) + carry;
        // new_valueの下半分を取り出して代入
        push_front(&buffer.unsigned_value, (uint32_t)new_value);
        // new_valueの上半分を桁上げとして保存
        carry = new_value >> kNumOfBitsInUInt;
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
    uint64_t carry = 0;
    // carry が残ることはない
    while(former_node != NULL || latter_node != NULL) {
        // 片方のリストが短くてもぬるぽしないようにsecurely~を使う
        // 繰り下がりのぶんを予め足しておく
        uint64_t new_value =
            ((uint64_t)1 << kNumOfBitsInUInt) +
            (uint64_t)securely_get_value(former_node) -
            (uint64_t)securely_get_value(latter_node) - carry;
        // new_valueの下半分を取り出して代入
        push_front(&buffer.unsigned_value, (uint32_t)new_value);
        // new_valueの上半分をみて繰り下がり判定
        carry = (new_value >> kNumOfBitsInUInt) == 0;
        // 片方のリストが短くてもぬるぽしないようにsecurely~を使う
        former_node = securely_get_prev_node(former_node);
        latter_node = securely_get_prev_node(latter_node);
    }
    remove_zero_nodes(&buffer);
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
    // 長さが等しいのでformerがNULLならlatterもNULLである
    while(former_node != NULL && former_node->key == latter_node->key) {
        former_node = former_node->next_node;
        latter_node = latter_node->next_node;
    }
    return securely_get_value(former_node) < securely_get_value(latter_node);
}

static int is_less_than_or_equal_to(LargeInt* former, LargeInt* latter) {
    Node* former_node = former->unsigned_value;
    Node* latter_node = latter->unsigned_value;
    while(former_node != NULL && latter_node != NULL) {

    }
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

    // null文字が入るので1足す
    large_int->hex_string = (char*)malloc(sizeof(char) * (string_length + 1));
    if(large_int->hex_string == NULL) {
        puts("Failed to allocate memory");
        exit(1);
    }
    Node* current_node = large_int->unsigned_value.head;
    for(int i = 0; i < get_length(&large_int->unsigned_value); i++) {
        // uint32_t一つぶんを文字列に変換する
        // 実は毎回出るnullバイトを塗りつぶしている
        snprintf(large_int->hex_string + i * kHexDigitsInUInt, kHexDigitsInUInt + 1, "%08x", current_node->key);
        current_node = current_node->next_node;
    }
}

// LargeIntのunsigned_valueからbinary_stringを更新する
void update_binary_string(LargeInt* large_int) {
    if(large_int->binary_string != NULL)
        free(large_int->binary_string);
    // 文字列の長さはノードの数とuint32のビット数
    int string_length = get_length(&large_int->unsigned_value) * kNumOfBitsInUInt;

    if(string_length == 0) {
        large_int->binary_string = (char*)malloc(sizeof(char) * 2);
        if(large_int->binary_string == NULL) {
            puts("Failed to allocate memory");
            exit(1);
        }
        large_int->binary_string[0] = '0';
        large_int->binary_string[1] = '\0';
    }

    // null文字が入るので1足す
    large_int->binary_string = (char*)malloc(sizeof(char) * (string_length + 1));
    if(large_int->binary_string == NULL) {
        fprintf(stderr, "Failed to allocate memory\n");
        exit(1);
    }

    // HACK: iとcurrent_node
    // 数値は右から1ビットずつ0か1かを判定するほうが簡単だが，文字列は左側から埋めていくほうが簡単(あまり変わらない気もするが)
    // 文字列を一旦左側から作って最後に反転させる
    Node* current_node = large_int->unsigned_value.last;
    for(int i = 0; current_node != NULL; current_node = current_node->prev_node) {
        uint32_t current_value = current_node->key;
        for(int j = 0; j < kNumOfBitsInUInt; j++) {
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
        // -1はnull文字を避けるため
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
