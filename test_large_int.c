#include <stdio.h>
#include "large_int.h"

int main() {
    LargeInt former;
    init_large_int(&former);
    print_binary(&former);
    hex_string_to_large_int("-0", &former);
    print_hex(&former);

    LargeInt latter;
    init_large_int(&latter);
    hex_string_to_large_int("1", &latter);
    print_hex(&latter);

    LargeInt result;
    init_large_int(&result);

    large_divide(&former, &latter, &result);
    print_hex(&result);

    // large_shift_right(&former);
    // update_hex_string(&former);
    // print_hex(&former);
    //
    // large_shift_right(&former);
    // update_hex_string(&former);
    // print_hex(&former);
    //
    // large_shift_left(&former);
    // update_hex_string(&former);
    // print_hex(&former);

    release_large_int(&former);
    release_large_int(&latter);
    release_large_int(&result);
    return 0;
}
