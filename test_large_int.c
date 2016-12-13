#include <stdio.h>
#include "large_int.h"

int main() {
    LargeInt former;
    init_large_int(&former);
    print_binary_string(&former);
    hex_string_to_large_int("-0", &former);
    print_hex_string(&former);

    LargeInt latter;
    init_large_int(&latter);
    hex_string_to_large_int("1", &latter);
    print_hex_string(&latter);

    LargeInt result;
    init_large_int(&result);

    large_divide(&former, &latter, &result);
    print_hex_string(&result);

    // large_shift_right(&former);
    // update_hex_string(&former);
    // print_hex_string(&former);
    //
    // large_shift_right(&former);
    // update_hex_string(&former);
    // print_hex_string(&former);
    //
    // large_shift_left(&former);
    // update_hex_string(&former);
    // print_hex_string(&former);

    release_large_int(&former);
    release_large_int(&latter);
    release_large_int(&result);
    return 0;
}
