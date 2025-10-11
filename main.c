#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "CArray.c"

DEFINE_ARRAY(Array_i32, i32);

int main() {
    init_allocator(temp, Megabytes(1));

    Array_i32 arr = {.allocator = temp};
    
    i32array_append(&arr ,12);

    printf("IM HERE really: %d\n", i32array_get_value(&arr, 0));
    
    return 0;
}
