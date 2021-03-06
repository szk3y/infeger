#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "large_int.h"

// HACK: formerとかlatterとかをいい感じの名前に

// 空のリストが渡される可能性を考慮すること

static void update_hex_string(LargeInt*);
static void update_binary_string(LargeInt*);
static void update_decimal_string(LargeInt*);
static char get_sign_char(LargeInt*);
static char* non_zero_starting_string(char*);

static void unsigned_hex_string_to_large_int(char* hex_string, LargeInt* large_int);
static uint32_t word_to_uint(char*, int beginning_index, int length); // 16進数文字列の32bit整数を32bit整数にする
static uint32_t hex_char_to_uint(char); // 16進数の文字を整数にして返す
static char decimal_digit_to_char(uint32_t); // 0~9の数字を文字に変換する
static void unsigned_decimal_string_to_large_int(char*, LargeInt*);
static uint32_t sub_string_to_uint32(char*, int, int);
static void uint32_to_large_int(uint32_t, LargeInt*);

// 符号の処理はこの2つの関数の後で行わなければならない
static void large_add(LargeInt* a, LargeInt* b, LargeInt* result); // 符号を気にせず result = a + b
static void large_sub(LargeInt* a, LargeInt* b, LargeInt* result); // 符号を気にせず result = a - b

static bool is_less_than(LargeInt* former, LargeInt* latter); // |former| < |latter|を返す
static bool is_less_than_or_equal_to(LargeInt* former, LargeInt* latter); // |former| <= |latter|を返す
static void swap(char*, char*); // aとbを入れ替える
static void reverse_string(char*); // 文字列を反転させる
static void remove_zero_nodes(LargeInt*); // 左側の不要な値0のノードをできるだけ消す
static void push_back_zero_nodes(LargeInt*, int); // LargeIntをuint32_t n個分左にシフトする
static void multiply_large_and_small(LargeInt* a, uint32_t b, LargeInt* result); // result = a * b
static void large_shift_left(LargeInt*); // LargeIntを1つだけ左論理シフトする
static void large_shift_right(LargeInt*); // LargeIntを1つだけ右論理シフトする

static void debug_print_string(char*);

// いらないかもしれない
// uint32_tの16進数での桁数
static const int kHexDigitInUInt = 8;
// uint32_tのビット数
static const int kNumOfBitsInUInt = 32;
// uint32_tですべて表現できる10進数の桁数
static const int kSafeDecimalDigitInUInt = 9;
// dummy_large_intで次のノードにかける数
static const int kNextKeyScale = 1000000000;

// LargeIntは最初に必ずこの関数を使って初期化する
void init_large_int(LargeInt* large_int) {
    large_int->is_negative = false;
    init_list(&large_int->unsigned_value);
    large_int->decimal_string = NULL;
    large_int->binary_string  = NULL;
    large_int->hex_string     = NULL;
}

void copy_large_int(LargeInt* origin, LargeInt* clone) {
    release_large_int(clone);
    copy_list(&origin->unsigned_value, &clone->unsigned_value);
}

void uint32_to_large_int(uint32_t num, LargeInt* large_int) {
    release_large_int(large_int);
    push_back(&large_int->unsigned_value, num);
}

void decimal_string_to_large_int(char* decimal_string, LargeInt* large_int) {
    release_large_int(large_int);
    large_int->is_negative = decimal_string[0] == '-';
    unsigned_decimal_string_to_large_int(decimal_string + large_int->is_negative, large_int);
}

// 10進数の数を小分けにしてlarge_intに足していく
static void unsigned_decimal_string_to_large_int(char* decimal_string, LargeInt* large_int) {
    release_large_int(large_int);
    // 10進数の数字をkSafeDecimalDigitInUIntと同じ桁数ずつノードにいれる
    List dummy_large_int;
    init_list(&dummy_large_int);

    // 残りの数をきれいに入れるために最初の処理で桁数を合わせる
    int first_length = strlen(decimal_string) % kSafeDecimalDigitInUInt;
    if(first_length != 0) {
        push_back(&dummy_large_int, sub_string_to_uint32(decimal_string, 0, first_length));
    }

    for(int i = first_length; i < (int)strlen(decimal_string); i = i + kSafeDecimalDigitInUInt) {
        push_back(&dummy_large_int, sub_string_to_uint32(decimal_string, i, kSafeDecimalDigitInUInt));
    }

    // kNextKeyScaleをintからLargeIntに変換
    LargeInt scale;
    init_large_int(&scale);
    push_back(&scale.unsigned_value, kNextKeyScale);

    int counter = 0;
    for(Node* node = dummy_large_int.last; node != NULL; node = node->prev_node) {
        // dummy_large_int->nodeからLargeIntに変換
        LargeInt tmp;
        init_large_int(&tmp);
        push_back(&tmp.unsigned_value, node->key);

        // 実際の値に直す
        for(int i = 0; i < counter; i++) {
            large_multiply(&tmp, &scale, &tmp);
        }
        large_add(large_int, &tmp, large_int);

        counter++;
        release_large_int(&tmp);
    }

    remove_zero_nodes(large_int);
    release_large_int(&scale);
    release_list(&dummy_large_int);
}

static uint32_t sub_string_to_uint32(char* string, int beginning_index, int length) {
    char buffer[kSafeDecimalDigitInUInt + 1];
    buffer[length] = '\0';
    strncpy(buffer, string + beginning_index, length);
    return (uint32_t)atol(buffer);
}

void hex_string_to_large_int(char* hex_string, LargeInt* large_int) {
    release_large_int(large_int);
    large_int->is_negative = hex_string[0] == '-';
    // 文字列が負のとき，iは1から始まる．
    unsigned_hex_string_to_large_int(hex_string + large_int->is_negative, large_int);
}

static void unsigned_hex_string_to_large_int(char* hex_string, LargeInt* large_int) {
    int length = kHexDigitInUInt;
    for(int i = 0; i < (int)strlen(hex_string); i = i + length) {
        // 最初の処理は残りの桁数がuint32_tで割り切れるように長さを調節する
        if(i == 0 && strlen(hex_string) % kHexDigitInUInt != 0) {
            length = strlen(hex_string) % kHexDigitInUInt;
        } else {
            length = kHexDigitInUInt;
        }
        push_back(&large_int->unsigned_value, word_to_uint(hex_string, i, length));
    }
    remove_zero_nodes(large_int);
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
            fprintf(stderr, "hex_char_to_uint: Invalid argument\n");
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
    bool is_negative = former->is_negative != latter->is_negative;
    LargeInt origin;
    init_large_int(&origin);
    LargeInt tmp;
    init_large_int(&tmp);
    // HACK: uint何個分左シフトするかを数える．他の方法を考える
    int counter = 0;
    for(Node* node = latter->unsigned_value.last; node != NULL; node = node->prev_node) {
        // formerとlatterの一部をかけて結果をtmpに保存する
        multiply_large_and_small(former, node->key, &tmp);
        push_back_zero_nodes(&tmp, counter);
        counter++;
        large_add(&origin, &tmp, &origin);
    }
    copy_large_int(&origin, clone);

    release_large_int(&tmp);
    release_large_int(&origin);
    clone->is_negative = is_negative;
}

// 1bitずつ筆算方式で求める
void large_divide(LargeInt* divident, LargeInt* divisor, LargeInt* result) {
    if(securely_get_value(divisor->unsigned_value.head) == 0) {
        fprintf(stderr, "large_divide: zero division\n");
        exit(1);
    }
    bool is_negative = divident->is_negative != divisor->is_negative;
    // この数字から引いていく
    LargeInt current_divident;
    init_large_int(&current_divident);
    copy_large_int(divident, &current_divident);
    // この数字を少しずつ小さくして引いていく
    // current_divident / 2 < current_divisor <= current_divident
    LargeInt current_divisor;
    init_large_int(&current_divisor);
    copy_large_int(divisor, &current_divisor);
    // 結果を一時的に保持する
    LargeInt quotient;
    init_large_int(&quotient);
    uint32_to_large_int(0, &quotient);
    // 引き算できる場合どのbitをオンにするかを示す
    LargeInt current_bit;
    init_large_int(&current_bit);
    uint32_to_large_int(1, &current_bit);

    // dividentより大きくなるまでcurrent_divisorをシフトして大きくする
    while(is_less_than_or_equal_to(&current_divisor, divident)) {
        large_shift_left(&current_divisor);
        large_shift_left(&current_bit);
    }

    // 本来の割る数よりも小さくなったら終了
    while(is_less_than_or_equal_to(divisor, &current_divisor)) {
        if(is_less_than_or_equal_to(&current_divisor, &current_divident)) {
            large_sub(&current_divident, &current_divisor, &current_divident);
            large_add(&quotient, &current_bit, &quotient);
        }
        large_shift_right(&current_divisor);
        large_shift_right(&current_bit);
    }
    copy_large_int(&quotient, result);
    result->is_negative = is_negative;

    release_large_int(&current_bit);
    release_large_int(&quotient);
    release_large_int(&current_divisor);
    release_large_int(&current_divident);
}

static void large_shift_left(LargeInt* large_int) {
    uint64_t carry = 0;
    // すべてのキーに対し左シフト
    for(Node* node = large_int->unsigned_value.last; node != NULL; node = node->prev_node) {
        uint64_t new_value = ((uint64_t)node->key << 1) + carry;
        node->key = (uint32_t)new_value;
        // はみ出た分を桁上げとして保存
        carry = new_value >> kNumOfBitsInUInt;
    }
    if(carry == 1) {
        push_front(&large_int->unsigned_value, 1);
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
    remove_zero_nodes(large_int);
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
        arg_node = securely_get_prev_node(arg_node);
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
    // carryだけ最後に残ることはない
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
static bool is_less_than(LargeInt* former, LargeInt* latter) {
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

// |former| <= |latter|を返す
// 最後以外はis_less_thanのコピー
static bool is_less_than_or_equal_to(LargeInt* former, LargeInt* latter) {
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
    return securely_get_value(former_node) <= securely_get_value(latter_node);
}

// LargeIntのunsigned_valueからhex_stringを更新する
static void update_hex_string(LargeInt* large_int) {
    if(large_int->hex_string != NULL)
        free(large_int->hex_string);

    if(is_empty(&large_int->unsigned_value)) {
        large_int->hex_string = (char*)malloc(sizeof(char) * 2);
        if(large_int->hex_string == NULL) {
            fprintf(stderr, "Failed to allocate memory\n");
            exit(1);
        }
        large_int->hex_string[0] = '0';
        large_int->hex_string[1] = '\0';
        return;
    }

    int string_length = get_length(&large_int->unsigned_value) * kHexDigitInUInt;

    // null文字が入るので1足す
    large_int->hex_string = (char*)malloc(sizeof(char) * (string_length + 1));
    if(large_int->hex_string == NULL) {
        fprintf(stderr, "Failed to allocate memory\n");
        exit(1);
    }
    Node* current_node = large_int->unsigned_value.head;
    for(int i = 0; i < get_length(&large_int->unsigned_value); i++) {
        // uint32_t一つぶんを文字列に変換する
        // 実は毎回出るnullバイトを塗りつぶしている
        snprintf(large_int->hex_string + i * kHexDigitInUInt, kHexDigitInUInt + 1, "%08x", current_node->key);
        current_node = current_node->next_node;
    }
}

// LargeIntのunsigned_valueからbinary_stringを更新する
static void update_binary_string(LargeInt* large_int) {
    if(large_int->binary_string != NULL)
        free(large_int->binary_string);

    if(is_empty(&large_int->unsigned_value)) {
        large_int->binary_string = (char*)malloc(sizeof(char) * 2);
        if(large_int->binary_string == NULL) {
            fprintf(stderr, "Failed to allocate memory\n");
            exit(1);
        }
        large_int->binary_string[0] = '0';
        large_int->binary_string[1] = '\0';
        return;
    }

    // 文字列の長さはノードの数とuint32のビット数
    int string_length = get_length(&large_int->unsigned_value) * kNumOfBitsInUInt;

    // null文字が入るので1足す
    large_int->binary_string = (char*)malloc(sizeof(char) * (string_length + 1));
    if(large_int->binary_string == NULL) {
        fprintf(stderr, "Failed to allocate memory\n");
        exit(1);
    }
    large_int->binary_string[string_length] = '\0';

    // HACK: iとcurrent_node
    // 数値は右から1ビットずつ0か1かを判定するほうが簡単だが，文字列は左側から埋めていくほうが簡単(あまり変わらない気もするが)
    // 文字列を一旦左側から作って最後に反転させる
    Node* current_node = large_int->unsigned_value.last;
    int i;
    for(i = 0; current_node != NULL; current_node = current_node->prev_node) {
        uint32_t current_value = current_node->key;
        for(int j = 0; j < kNumOfBitsInUInt; j++) {
            if(current_value % 2 == 0) {
                large_int->binary_string[i] = '0';
            } else {
                large_int->binary_string[i] = '1';
            }
            current_value = current_value >> 1;
            i++;
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

int get_digit(LargeInt* large_int) {
    if(is_empty(&large_int->unsigned_value)) {
        return 1;
    }
    // 10，100などの値を保持する
    LargeInt scale;
    init_large_int(&scale);
    uint32_to_large_int(10, &scale);

    LargeInt ten;
    init_large_int(&ten);
    uint32_to_large_int(10, &ten);

    int counter = 1;
    while(is_less_than_or_equal_to(&scale, large_int)) {
        counter++;
        large_multiply(&scale, &ten, &scale);
    }

    release_large_int(&ten);
    release_large_int(&scale);
    return counter;
}

static char decimal_digit_to_char(uint32_t num) {
    switch(num) {
        case 0:
            return '0';
        case 1:
            return '1';
        case 2:
            return '2';
        case 3:
            return '3';
        case 4:
            return '4';
        case 5:
            return '5';
        case 6:
            return '6';
        case 7:
            return '7';
        case 8:
            return '8';
        case 9:
            return '9';
        default:
            fprintf(stderr, "decimal_digit_to_char: Invalid argument\n");
            exit(1);
    }
}

static void update_decimal_string(LargeInt* large_int) {
    if(large_int->decimal_string != NULL)
        free(large_int->decimal_string);
    int digit = get_digit(large_int);
    large_int->decimal_string = (char*)malloc(sizeof(char) * (digit + 1));
    if(large_int->decimal_string == NULL) {
        fprintf(stderr, "Failed to allocate memory\n");
        exit(1);
    }
    // 最後尾にnull文字をいれておく
    large_int->decimal_string[digit] = '\0';

    if(is_empty(&large_int->unsigned_value)) {
        large_int->decimal_string[0] = '0';
        return;
    }

    LargeInt scale;
    init_large_int(&scale);
    uint32_to_large_int(1, &scale);

    LargeInt ten;
    init_large_int(&ten);
    uint32_to_large_int(10, &ten);

    LargeInt clone;
    init_large_int(&clone);
    copy_large_int(large_int, &clone);

    // まずはscaleを10進数でlarge_intと同じ桁数にする
    for(int i = 0; i < digit - 1; i++) {
        large_multiply(&scale, &ten, &scale);
    }

    // 10進数の各桁で何回引き算ができるかを調べて文字列にする
    // 割り算を使っても良いかもしれない．ただし，答えはLargeIntになるので変換しなければならない．
    int index = 0;
    while(scale.unsigned_value.head->key != 0) {
        uint32_t counter = 0;
        while(is_less_than_or_equal_to(&scale, &clone)) {
            large_sub(&clone, &scale, &clone);
            counter++;
        }
        large_int->decimal_string[index] = decimal_digit_to_char(counter);
        index++;
        large_divide(&scale, &ten, &scale);
    }

    release_large_int(&clone);
    release_large_int(&ten);
    release_large_int(&scale);
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

void print_hex_string(LargeInt* large_int) {
    update_hex_string(large_int);
    printf("%c0x", get_sign_char(large_int));
    puts(non_zero_starting_string(large_int->hex_string));
}

void print_binary_string(LargeInt* large_int) {
    update_binary_string(large_int);
    printf("%c0b", get_sign_char(large_int));
    puts(non_zero_starting_string(large_int->binary_string));
}

void print_decimal_string(LargeInt* large_int) {
    update_decimal_string(large_int);
    printf("%c0d", get_sign_char(large_int));
    puts(non_zero_starting_string(large_int->decimal_string));
}

// 最初の0でない文字のポインタを返す．全て0なら最後の文字を指すポインタを返す．
static char* non_zero_starting_string(char* string) {
    for(int i = 0; i < (int)strlen(string); i++) {
        if(string[i] != '0') {
            return string + i;
        }
    }
    // 文字列の末尾
    return string + strlen(string) - 1;
}

static char get_sign_char(LargeInt* large_int) {
    if(large_int->is_negative && securely_get_value(large_int->unsigned_value.head) != 0)
        return '-';
    else
        return '+';
}

void debug_print_large_int(LargeInt* large_int) {
    printf("LargeInt: %p\n", large_int);
    printf("  is_negative:    %d\n", large_int->is_negative);
    printf("  decimal_string: ");
    print_address(large_int->decimal_string);
    debug_print_string(large_int->decimal_string);
    printf("  binary_string:  ");
    print_address(large_int->binary_string);
    debug_print_string(large_int->binary_string);
    printf("  hex_string:     ");
    print_address(large_int->hex_string);
    debug_print_string(large_int->hex_string);
    debug_print_list(&large_int->unsigned_value);
}

static void debug_print_string(char* string) {
    if(string == NULL)
        return;
    printf("    %s\n", string);
}
