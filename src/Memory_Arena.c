#pragma once

#include "my_defines.h"
#include <stdlib.h> // For calloc.

// Right now it's just arena, but we keep possibility of different arenas. 
// It's just sounds more nice in context of default_arena, where we could assign global arena arena for time.
// And everyone will be using this default arena when nothing else is specified.
typedef struct Memory_Arena {
    i32 reserved;
    i32 watermark;
    char *start;
} Memory_Arena;

#define HEAP_ALLOCATOR NULL

// NULL on default arena means it will be just malloc.
Memory_Arena temp_arena    = {0};
Memory_Arena *temp = &temp_arena;
Memory_Arena state_arena   = {0};

void init_arena(Memory_Arena *arena, size_t size) {
    assert(arena->reserved <= 0 && arena->watermark == 0 && "On initing arena should be free from all chains");

    arena->reserved = size;
    arena->watermark = 0;
    arena->start = (char *)calloc(1, size);
}

void *alloc(Memory_Arena *arena, size_t size) {
    if (!arena) return (void *)calloc(1, size);
    
    assert(arena->watermark + size < arena->reserved && "We don't handle situation where memory arena consumed more than it could handle. Alloc more on the start or think about your behaviour.");
    
    u8 *result = arena->start + arena->watermark;
    memset(result, 0, size);
    arena->watermark += size;
    
    return result;
}

void clear_arena(Memory_Arena *arena) {
    arena->watermark = 0;
}

void free_arena(Memory_Arena *arena) {
    free(arena->start);
}
    
void free_data_in_arena(Memory_Arena *arena, void *data) {
    // If arena is null - data was allocated via default heap allocator.
    if (!arena) {
        free(data);
    }
}
