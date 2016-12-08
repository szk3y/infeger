#include <stdio.h>
#include "large_int.h"

int main() {
    LargeInt former;
    init_large_int(&former);
    hex_string_to_large_int("-1ffffffff", &former);
    update_hex_string(&former);
    print_hex(&former);

    LargeInt latter;
    init_large_int(&latter);
    hex_string_to_large_int("1", &latter);
    update_hex_string(&latter);
    print_hex(&latter);

    large_plus(&former, &latter, &latter);
    update_hex_string(&latter);
    print_hex(&latter);

    release_large_int(&former);
    release_large_int(&latter);
    return 0;
}
