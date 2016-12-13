#include <stdio.h>
#include <stdlib.h>
#include "large_int.h"

void do_operation(LargeInt*, char, LargeInt*, LargeInt*);

int main(int argc, char** argv) {
    if(argc != 4) {
        fprintf(stderr, "Usage: %s <operand> <operator> <operand>\n", argv[0]);
        fprintf(stderr, "Available operators: +, -, x, /\n");
        exit(1);
    }

    LargeInt operand1;
    init_large_int(&operand1);
    hex_string_to_large_int(argv[1], &operand1);

    LargeInt operand2;
    init_large_int(&operand2);
    hex_string_to_large_int(argv[3], &operand2);

    LargeInt result;
    init_large_int(&result);
    do_operation(&operand1, argv[2][0], &operand2, &result);
    print_hex_string(&result);
    print_binary_string(&result);
    print_decimal_string(&result);
    printf("Digit: %d\n", get_digit(&result));

    release_large_int(&result);
    release_large_int(&operand2);
    release_large_int(&operand1);
    return 0;
}

void do_operation(LargeInt* operand1, char operator, LargeInt* operand2, LargeInt* result) {
    switch(operator) {
        case '+':
            large_plus(operand1, operand2, result);
            break;
        case '-':
            large_minus(operand1, operand2, result);
            break;
        case 'x':
            large_multiply(operand1, operand2, result);
            break;
        case '/':
            large_divide(operand1, operand2, result);
            break;
        default:
            fprintf(stderr, "Invalid operator\n");
            exit(1);
    }
}
