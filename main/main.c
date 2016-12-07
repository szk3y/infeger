#include <stdio.h>
#include "large_int.h"

int main() {
    LargeInt large1;
    init_large_int(&large1);
    hex_string_to_large_int("1ffffffff", &large1);
    update_hex_string(&large1);
    printf("0x");
    print_hex(&large1);

    puts("+");

    LargeInt large2;
    init_large_int(&large2);
    hex_string_to_large_int("1", &large2);
    update_hex_string(&large2);
    printf("0x");
    print_hex(&large2);

    puts("=");

    LargeInt large3;
    init_large_int(&large3);
    large_add(&large1, &large2, &large3);
    update_hex_string(&large3);
    printf("0x");
    print_hex(&large3);

    release_large_int(&large1);
    release_large_int(&large2);
    release_large_int(&large3);
    return 0;
}
