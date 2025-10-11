#pragma once

#include "my_defines.h"
#include <stdlib.h> // For calloc.

// Right now it's just arena, but we keep possibility of different allocators. 
// It's just sounds more nice in context of default_allocator, where we could assign global arena allocator for time.
// And everyone will be using this default allocator when nothing else is specified.
typedef struct Allocator {
    i32 reserved;
    i32 watermark;
    char *start;
} Allocator;

#define HEAP_ALLOCATOR NULL

// NULL on default allocator means it will be just malloc.
Allocator temp_allocator    = {0};
Allocator *temp = &temp_allocator;
Allocator state_allocator   = {0};

void init_allocator(Allocator *allocator, size_t size) {
    assert(allocator->reserved <= 0 && allocator->watermark == 0 && "On initing allocator should be free from all chains");

    allocator->reserved = size;
    allocator->watermark = 0;
    allocator->start = (char *)calloc(1, size);
}

char *alloc(Allocator *allocator, size_t size) {
    if (!allocator) return (char *)calloc(1, size);
    
    assert(allocator->watermark + size < allocator->reserved && "We don't handle situation where memory arena consumed more than it could handle. Alloc more on the start or think about your behaviour.");
    
    char *result = allocator->start + allocator->watermark;
    for (i32 i = 0; i < size; i++) {
        result[i] = 0;   
    }
    allocator->watermark += size;
    
    return result;
}

void clear_allocator(Allocator *allocator) {
    allocator->watermark = 0;
}

void free_allocator(Allocator *allocator) {
    free(allocator->start);
}
    
void free_data_in_allocator(Allocator *allocator, void *data) {
    // Currently allocator is just Memory Arena, so we can't just free data in it.
    // That's why we call default 'free' when allocator is NULL - that's mean it was allocated with just calloc.
    if (!allocator) {
        free(data);
    }
}
