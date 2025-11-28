#pragma once

#include "Allocator.c"

#define for_array(index, array) for (i32 index = 0; index < (array)->count; index++)
#define for_array_backwards(index, array) for (i32 index = (array)->count - 1; index >= 0; index--)

inline void grow_if_need(void **data, size_t element_size, i32 *capacity, i32 current_count, i32 appended_count) {
    i32 new_count = current_count + appended_count;
    if (new_count > *capacity) {
        void *old_data = *data;
        
        while (new_count > *capacity) {
            if (*capacity == 0) {
                *capacity = 8;
            } else {
                *capacity *= 2;
            }
        }
        
        *data = calloc(1, *capacity * element_size);
        // old_data could be not present if we growing for the first time (so data was null).
        if (old_data) {
            memcpy(*data, old_data, current_count * element_size);
            free(old_data);
        }
        
    }
}

#define DEFINE_ARRAY(Array_Name, T)                                                                                        \
typedef struct {                                                                                                           \
    T *data;                                                                                                               \
    Allocator *allocator;                                                                                                  \
                                                                                                                           \
    i32 count;                                                                                                             \
    i32 capacity;                                                                                                          \
} Array_Name;                                                                                                              \
void T##array_free_data(Array_Name *array) {                                                                               \
    if (array->data) {                                                                                                     \
        free(array->data);                                                                                                 \
        array->data = NULL;                                                                                                \
    } else {                                                                                                               \
        assert(array->capacity == 0 && array->count == 0);                                                                 \
    }                                                                                                                      \
    array->capacity = 0;                                                                                                   \
    array->count = 0;                                                                                                      \
}                                                                                                                          \
                                                                                                                           \
void T##array_clear(Array_Name *array) {                                                                                   \
    array->count = 0;                                                                                                      \
}                                                                                                                          \
                                                                                                                           \
inline T *T##array_last(Array_Name *array) {                                                                               \
    return &array->data[array->count - 1];                                                                                 \
}                                                                                                                          \
inline T T##array_last_value(Array_Name *array) {                                                                          \
    return array->data[array->count - 1];                                                                                  \
}                                                                                                                          \
inline T *T##array_get(Array_Name *array, i32 index) {                                                                     \
    assert((index >= 0 && index < array->count) && "Index out of bounds!");                                                \
                                                                                                                           \
    return &array->data[index];                                                                                            \
}                                                                                                                          \
                                                                                                                           \
inline T T##array_get_value(Array_Name *array, i32 index) {                                                                \
    assert((index >= 0 && index < array->count) && "Index out of bounds!");                                                \
                                                                                                                           \
    return array->data[index];                                                                                             \
}                                                                                                                          \
                                                                                                                           \
T *T##array_append(Array_Name *array, T value) {                                                                           \
    grow_if_need((void **)(&array->data), sizeof(T), &array->capacity, array->count, 1);                                   \
                                                                                                                           \
    array->data[array->count] = value;                                                                                     \
    array->count += 1;                                                                                                     \
                                                                                                                           \
    return T##array_last(array);                                                                                           \
}                                                                                                                          \
                                                                                                                           \
void T##array_append_another_array(Array_Name *array, Array_Name *another_array) {                                         \
    for_array(i, another_array) {                                                                                          \
        T##array_get_value(another_array, i);                                                                              \
    }                                                                                                                      \
}                                                                                                                          \
                                                                                                                           \
void T##array_just_decrease_count(Array_Name *array) {                                                                     \
    array->count -= 1;                                                                                                     \
    assert(array->count >= 0);                                                                                             \
}                                                                                                                          \
                                                                                                                           \
b32 T##array_values_equal(Array_Name *array, Array_Name *another_array) {                                                  \
    if (array->count != another_array->count) return false;                                                                \
                                                                                                                           \
    for_array(i, array) {                                                                                                  \
        if (*T##array_get(array, i) != *T##array_get(another_array, i)) {                                                  \
            return false;                                                                                                  \
        }                                                                                                                  \
    }                                                                                                                      \
                                                                                                                           \
    return true;                                                                                                           \
}                                                                                                                          \
/* This function will not grow an array, because it's for situations where we want to reuse previously assigned values. */ \
T *T##array_increase_count_and_get_last(Array_Name *array) {                                                               \
    array->count += 1;                                                                                                     \
    assert(array->count <= array->capacity);                                                                               \
    return T##array_last(array);                                                                                           \
}                                                                                                                          \
                                                                                                                           \
void copy_values(Array_Name *array, Array_Name *another_array) {                                                           \
    T##array_clear(array);                                                                                                 \
    T##array_append_another_array(array, another_array);                                                                   \
}                                                                                                                          \
                                                                                                                           \
T *T##array_insert(Array_Name *array, T value, i32 index) {                                                                \
    /* We could think about growing array if insert index exeeds current array->capacity, but curently  */                 \
    /* I think we should keep it simple and allow insert only on existing places. */                                       \
    assert(index >= 0 && index < array->count);                                                                            \
                                                                                                                           \
    array->data[index] = value;                                                                                            \
                                                                                                                           \
    return &array->data[index];                                                                                            \
}                                                                                                                          \
                                                                                                                           \
void T##array_remove(Array_Name *array, i32 index) {                                                                       \
    assert((index >= 0 && index < array->count) && "Index out of bounds!");                                                \
                                                                                                                           \
    if (index == array->count - 1) {                                                                                       \
        array->count -= 1;                                                                                                 \
        return;                                                                                                            \
    }                                                                                                                      \
                                                                                                                           \
    memmove(T##array_get(array, index), T##array_get(array, index+1), sizeof(T) * (array->count - index - 1));             \
    array->count--;                                                                                                        \
}                                                                                                                          \
                                                                                                                           \
inline void T##array_remove_first_encountered(Array_Name *array, T *value) {                                               \
    i32 index = T##array_find(array, value);                                                                               \
    if (index >= 0) T##array_remove(array, index);                                                                         \
}                                                                                                                          \
inline void T##array_remove_first_encountered_value(Array_Name *array, T value) {                                          \
    T##array_remove_first_encountered(array, &value);                                                                      \
}                                                                                                                          \
                                                                                                                           \
inline void T##array_remove_all_encountered(Array_Name *array,T *value) {                                                  \
    for_array(i, array) {                                                                                                  \
        if (*T##array_get(array, i) == *value) {                                                                           \
            T##array_remove(array, i);                                                                                     \
            i--;                                                                                                           \
        }                                                                                                                  \
    }                                                                                                                      \
}                                                                                                                          \
inline void T##array_remove_all_encountered_value(Array_Name *array, T value) {                                            \
    remove_all_encountered(&value);                                                                                        \
}                                                                                                                          \
                                                                                                                           \
void T##array_remove_first_half(Array_Name *array){                                                                        \
    i32 half_count = (i32)((f32)array->count * 0.5f);                                                                      \
    i32 even_correction = array->count % 2 == 0 ? -1 : 0;                                                                  \
    memcpy(T##array_get(array, 0), T##array_get(array, half_count + even_correction), half_count * sizeof(T));             \
                                                                                                                           \
    array->count = half_count;                                                                                             \
}                                                                                                                          \
                                                                                                                           \
inline b32 T##array_contains(Array_Name *array, T *to_find) {                                                              \
    for_array(i, array) {                                                                                                  \
        if (*T##array_get(array, i) == *to_find) {                                                                         \
            return true;                                                                                                   \
        }                                                                                                                  \
    }                                                                                                                      \
                                                                                                                           \
    return false;                                                                                                          \
}                                                                                                                          \
                                                                                                                           \
inline b32 T##array_contains_value(Array_Name *array, T to_find) {                                                         \
    return T##array_contains(array, &to_find);                                                                             \
}                                                                                                                          \
                                                                                                                           \
inline b32 T##array_contains_at_least_one(Array_Name *array, Array_Name *another_array) {                                  \
    for_array(i, another_array) {                                                                                          \
        if (T##array_contains(array, T##array_get(another_array, i))) {                                                    \
            return true;                                                                                                   \
        }                                                                                                                  \
    }                                                                                                                      \
                                                                                                                           \
    return false;                                                                                                          \
}                                                                                                                          \
                                                                                                                           \
/* Returns a array of elements that present in one and not in another. */                                                  \
/* Does not tries to look at duplicates, checks only for unique elements. */                                               \
inline Array_Name T##array_get_unique_elements_differences(Array_Name *array, Array_Name *another_array) {                 \
    Array_Name result = {.allocator = temp};                                                                               \
                                                                                                                           \
    Array_Name *biggest = array->count > another_array->count ? array : another_array;                                     \
    Array_Name *smallest = array->count > another_array->count ? another_array : array;                                    \
    for_array(i, biggest) {                                                                                                \
        if (!T##array_contains(smallest, T##array_get(biggest, i))) {                                                      \
            T##array_append(&result, T##array_get_value(biggest, i));                                                      \
        }                                                                                                                  \
    }                                                                                                                      \
                                                                                                                           \
    return result;                                                                                                         \
}                                                                                                                          \
                                                                                                                           \
inline i32 T##array_find(Array_Name *array, T *to_find) {                                                                  \
    for_array(i, array) {                                                                                                  \
        if (*T##array_get(array, i) == *to_find) {                                                                         \
            return i;                                                                                                      \
        }                                                                                                                  \
    }                                                                                                                      \
                                                                                                                           \
    return -1;                                                                                                             \
}                                                                                                                          \
inline i32 T##array_find_value(Array_Name *array, T to_find) {                                                             \
    return T##array_find(array, &to_find);                                                                                 \
}                                                                                                                          \
                                                                                                                           \
inline i32 T##array_find_from(Array_Name *array, T *to_find, i32 from) {                                                   \
    for (u32 i = from ; i < array->count; i++) {                                                                           \
        if (*T##array_get(array, i) == *to_find) {                                                                         \
            return i;                                                                                                      \
        }                                                                                                                  \
    }                                                                                                                      \
                                                                                                                           \
    return -1;                                                                                                             \
}                                                                                                                          \
inline i32 T##array_find_from_value(Array_Name *array, T to_find, i32 from) {                                              \
    return T##array_find_from(array, &to_find, from);                                                                      \
}                                                                                                                          \
                                                                                                                           \
