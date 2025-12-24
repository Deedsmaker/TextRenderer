#pragma once

#include <stdarg.h>
#include <stdlib.h>
#include "Memory_Arena.c"
#include "my_defines.h"

#define MEDIUM_STR_LEN 1024
#define MAX_TEMP_LINES 32

typedef struct String_Builder {
    Memory_Arena *arena;
    
    char *data;
    i32 count;
    i32 capacity;
} String_Builder;

global_variable char temp_lines[MAX_TEMP_LINES][MEDIUM_STR_LEN];
global_variable i32 temp_line_index = 0;

void increment_temp_line_index(){
    temp_line_index = (temp_line_index + 1) % MAX_TEMP_LINES;   
}

size_t str_len(const char *line){
    size_t result = 0;   
    while (line && line[result]){
        result++;
    }
    
    return result;
}

void mem_copy_back(void *dest, void *source, size_t bytes){
    char *new_dest   = (char*)dest;
    char *new_source = (char*)source;
    for (int i = bytes - 1; i >= 0; --i){
        new_dest[i] = new_source[i];
        //printf("%s\n", new_dest[i]);
    }
}

void mem_copy(void *dest, void *source, size_t bytes){
    char *new_dest   = (char*)dest;
    char *new_source = (char*)source;
    for (int i = 0; i < bytes; i++){
        new_dest[i] = new_source[i];
        //printf("%s\n", new_dest[i]);
    }
}

void mem_set(void *dest, u8 value, size_t bytes){
    char *new_dest   = (char*)dest;
    for (int i = 0; i < bytes; i++){
        new_dest[i] = value;
    }
}

void str_copy(char *dest, const char *source){
    int len = str_len(source);
    mem_copy((void*)dest, (void*)source, len * sizeof(char));
    dest[len] = '\0';

    // size_t bytes = str_len(source);
    // for (size_t i = bytes; i > 0; --i){
    //     dest[i - 1] = source[i - 1];
    //     //printf("%s\n", new_dest[i]);
    // }
}

inline char to_lower(char ch){
    return ch | 0x20;
}

inline bool is_digit(char ch){
    return ch >= '0' && ch <= '9';
}

bool str_start_with(const char *str, const char *start_with){
    int len = str_len(str);
    int start_with_len = str_len(start_with);
    
    if (start_with_len > len){
        return false;
    }
    
    for (int i = 0; i < start_with_len; i++){
        if (to_lower(str[i]) != to_lower(start_with[i])){
            return false;
        }
    }
    
    return true;
}

int str_find(const char *line, char symbol){
    size_t line_len = str_len(line);
    
    for (int i = 0; i < line_len; i++){
        if (line[i] == symbol){
            return i;
        }
    }
    
    return line_len;
}

i32 get_index_of_substring_start(const char *line, const char *substring){
    size_t line_length = str_len(line);
    size_t substring_length = str_len(substring);
    
    if (substring_length > line_length) return -1;
    
    i32 substring_start_index = -1;
    i32 found_substring_in_line_count = 0;
    
    for (i32 i = 0; i < line_length; i++) {
        if (substring_start_index == -1 && line[i] == substring[found_substring_in_line_count]) {
            substring_start_index = i;
            found_substring_in_line_count = 1;
            continue;
        }
        
        if (substring_start_index >= 0) {
            if (substring[found_substring_in_line_count] == line[i]) {
                found_substring_in_line_count += 1;
                
                if (found_substring_in_line_count >= substring_length) {
                    assert(found_substring_in_line_count == substring_length);
                    break;
                }
            } else {
                substring_start_index = -1;
                found_substring_in_line_count = 0;
            }
        }
    }
    
    if (found_substring_in_line_count >= substring_length) {
        assert(found_substring_in_line_count == substring_length);
        assert(substring_start_index > -1);        
        return substring_start_index;
    } else {
        return -1;
    }
}

void substring_after_line(char *line, const char *excluded){
    int start_index = 0;
    
    int line_len = str_len(line);
    int excluded_len = str_len(excluded);
    
    if (excluded_len > line_len){
        return;
    }
    
    mem_copy(line, line + excluded_len, (line_len - excluded_len) * sizeof(char));
    line[line_len - excluded_len] = '\0';
}
void substring_before_line(char *line, const char *excluded){
    i32 i = get_index_of_substring_start(line, excluded);
    
    if (i > -1) {
        line[i] = '\0';
    }
}

void substring_before_symbol(char *line, char symbol){
    int start_index = 0;
    
    int line_len = str_len(line);
    
    int copy_len = str_find(line, symbol);
    if (copy_len == line_len){
        printf("EWATAHTELLL\n");
    } else{
        mem_copy(line, line, str_find(line, symbol) * sizeof(char));
        line[copy_len] = '\0';
    }
}
    

char* get_substring_before_symbol(const char *line, char symbol){
    int start_index = 0;
    
    char *buffer = temp_lines[temp_line_index];
    buffer[0] = 0;
    
    i32 symbol_index = str_find(line, symbol);
    mem_copy(buffer, (void*)line, symbol_index * sizeof(char));
    
    // int line_len = str_len(line);
    buffer[symbol_index] = '\0';
    
    increment_temp_line_index();
    return buffer;
}

bool str_end_with(char *str, const char *end_with){
    int len = str_len(str);
    int end_with_len = str_len(end_with);
    
    if (end_with_len > len){
        return false;
    }
    
    for (int i = 0; i < end_with_len; i++){
        if (str[len - end_with_len + i] != end_with[i]){
            return false;
        }
    }
    
    return true;
}

bool str_contains_old(const char *str, const char *contains){
    size_t len = str_len(str);
    size_t contains_len = str_len(contains);
    
    size_t match_count = 0;
    //bool found_first_char = false;
    
    if (contains_len > len){
        return false;
    }
    
    for (int i = 0; i < len && match_count < contains_len; i++){
        if (match_count == 0 && len - i < contains_len){
            return false;
        }
    
        if (str[i] == contains[match_count]){
            match_count++;
        } else{
            //return false;
            match_count = 0;
        }
    }    
    
    return match_count == contains_len;
}

// This implementation seems more readable/understandable for some reason. 
// Maybe because we're not using more variables (like match_count in old one).
bool str_contains_new(const char *string, const char *contains) {
    size_t string_length = str_len(string);
    size_t contains_length = str_len(contains);
    
    if (contains_length > string_length) return 0;
    
    for (int i = 0; i < string_length; i++) {
        if (string[i] != contains[0]) continue;
        if (contains_length == 1) return true;
        // // So if it's last symbol in string and contains length is more than 1 we can't go any further.
        // if (i == string_length - 1) return 0;
        if (string_length - i < contains_length) return false; // So contains is just has more symbols than left in string.
        
        i += 1;
        for (int j = 1; j < contains_length; j++) {
            if (string[i] == contains[j]) {
                if (j == contains_length - 1) return true; // So we're checked every contains symbol and did not exit this loop - that means contains is fully persist in string.
                
                i += 1;
                // if (i >= string_length) return false; // String ended before we've got through all of contains.
            } else {
                break;
            }
        }
    }
    
    return false;
}

inline bool str_contains(const char *string, const char *contains) {
#if DEBUG_BUILD
    bool old_result = str_contains_old(string, contains);
    bool new_result = str_contains_new(string, contains);
    assert(old_result == new_result && "New str_contains implementation is different from old!");
    return new_result;
#else
    return str_contains_new(string, contains);
#endif
}

// bool str_contains(const char *str, const char *contains){
//     return str_contains_const(str, contains);
// }

// bool str_contains(char *str, char *contains){
//     return str_contains_const(str, contains);
// }

bool str_equal(const char *first, const char *second){
    int len1 = str_len(first);
    int len2 = str_len(second);
    
    if (len1 != len2 || len1 == 0) return false;
    
    for (int i = 0; i < len1; i++){
        if (first[i] != second[i]) return false;
    }
    
    return true;
}

const char *tprintf(const char *text, ...){ 
    char *current_buffer = temp_lines[temp_line_index];
    // mem_set(current_buffer, 0, MEDIUM_STR_LEN);
    current_buffer[0] = 0;
    
    va_list args;
    va_start(args, text);
    i32 byte_count = vsnprintf(current_buffer, MEDIUM_STR_LEN, text, args);
    va_end(args);
    
    if (byte_count >= MEDIUM_STR_LEN){
        char *trunc_buffer = current_buffer + MEDIUM_STR_LEN - 15;
        sprintf(trunc_buffer, "OVERFLOWWstati");
    }
    
    increment_temp_line_index();
    
    return current_buffer;
}

i32 to_i32(const char *text){ 
    i32 value = 0;
    i32 sign = 1;
    
    if (text[0] == '-') sign = -1;
    while ((text[0] == '+') || (text[0] == '-')){
        text++;
    }
    
    for (i32 i = 0; ((text[i] >= '0') && (text[i] <= '9')); i++){
        value = value * 10 + (i32)(text[i] - '0');
    }
    
    return value * sign;
}

u64 to_u64(const char *text){
    u64 value = 0;
    u64 sign = 1;
    
    if (text[0] == '-') sign = -1;
    while ((text[0] == '+') || (text[0] == '-')){
        text++;
    }
    
    for (u64 i = 0; ((text[i] >= '0') && (text[i] <= '9')); i++){
        value = value * 10 + (u64)(text[i] - '0');
    }
    
    return value * sign;
}

f32 to_f32(const char *text){
    f32 value = 0.0f;
    f32 sign = 1.0f;
    
    if (text[0] == '-') sign = -1.0f;
    while (text[0] == '+' || text[0] == '-'){
        text++;
    }
    
    i32 i = 0;
    for (; (text[i] >= '0' && text[i] <= '9'); i++){
        value = value * 10.0f + (f32)(text[i] - '0');
    }
    
    if (text[i++] == '.'){
        f32 divisor = 10.0f;
        for (; text[i] >= '0' && text[i] <= '9'; i++){
            value += ((f32)(text[i] - '0')) / divisor;
            divisor *= 10;
        }
    }
    
    return value * sign;
}

typedef struct String {
    char *data;
    Memory_Arena *arena;
    i32 count;
} String;

bool string_equal(String first, String second) {
    if (first.count != second.count) return false;
        
    for (int i = 0; i < first.count; i++){
        if (first.data[i] != second.data[i]) return false;
    }
    
    return true;    
}

bool string_equal_str(String string, const char *str){
    int len = str_len(str);

    if (string.count != len) return false;
    
    for (int i = 0; i < string.count; i++){
        if (string.data[i] != str[i]) return false;
    }
    
    return true;
}

void string_free_data(String *string){
    // If arena is persist we don't free that one because Memory_Arena is just arena for the time being.
    if (!string->arena){
        free(string->data);
        string->count = 0;
    }
}

i32 to_i32_string(String string){
    i32 value = 0;
    i32 sign = 1;
    
    i32 index = 0;
    if (string.data[0] == '-') sign = -1;
    while ((string.data[index] == '+' || string.data[index] == '-') && index < string.count){
        index += 1;
    }
    
    for (i32 i = index; ((string.data[i] >= '0') && (string.data[i] <= '9')) && i < string.count; i++){
        value = value * 10 + (i32)(string.data[i] - '0');
    }
    
    return value * sign;
}

u64 to_u64_string(String string){
    u64 value = 0;
    u64 sign = 1;
    
    i32 index = 0;
    
    if (string.data[0] == '-') sign = -1;
    while ((string.data[index] == '+' || string.data[index] == '-') && index < string.count){
        index += 1;
    }
    
    for (u64 i = index; ((string.data[i] >= '0') && (string.data[i] <= '9')) && i < string.count; i++){
        value = value * 10 + (u64)(string.data[i] - '0');
    }
    
    return value * sign;
}
u8 to_u8_string(String string){
    u8 value = 0;
    u8 sign = 1;
    
    i32 index = 0;
    
    if (string.data[0] == '-') sign = -1;
    while ((string.data[index] == '+' || string.data[index] == '-') && index < string.count){
        index += 1;
    }
    
    for (u8 i = index; ((string.data[i] >= '0') && (string.data[i] <= '9')) && i < string.count; i++){
        value = value * 10 + (u8)(string.data[i] - '0');
    }
    
    return value * sign;
}

f32 to_f32_string(String string){
    f32 value = 0.0f;
    f32 sign = 1.0f;
    
    i32 index = 0;
    
    if (string.data[0] == '-') sign = -1.0f;
    while ((string.data[index] == '+' || string.data[index] == '-') && index < string.count){
        index += 1;
    }
    
    i32 i = index;
    for (; (string.data[i] >= '0' && string.data[i] <= '9') && i < string.count; i++){
        value = value * 10.0f + (f32)(string.data[i] - '0');
    }
    
    if (i < string.count && string.data[i++] == '.'){
        f32 divisor = 10.0f;
        for (; string.data[i] >= '0' && string.data[i] <= '9' && i < string.count; i++){
            value += ((f32)(string.data[i] - '0')) / divisor;
            divisor *= 10;
        }
    }
    
    return value * sign;
}

const char *c_str(String string) {
    // Currently we null-terminate strings by default, so returning just data.
    if (string.count == 0) return "\0";
    return string.data;
}

String string(Memory_Arena *arena, const char *text, ...){
    String result_string = {.arena = arena};
    
    if (!text) {
        return result_string;
    }
    
    va_list args;
    va_start(args, text);
    i32 byte_count = vsnprintf(result_string.data, 0, text, args);
    result_string.data = alloc(result_string.arena, byte_count + 1);
    vsnprintf(result_string.data, byte_count + 1, text, args);
    va_end(args);
    
    result_string.count = byte_count;
    
    return result_string;
}

String copy_string(String other, Memory_Arena *arena) {
    String s = {.arena = arena};
    
    s.data = alloc(s.arena, other.count + 1);
    mem_copy(s.data, other.data, other.count);
    s.count = other.count;
    s.data[s.count] = 0;
    
    return s;
}

// end_index included.
String make_substring(String original_string, int start_index, int end_index, Memory_Arena *arena) {
    String new_string = {.arena = arena};
    
    if (end_index < start_index) {
        return new_string;
    }
    
    new_string.count = end_index - start_index + 1; // +1 because if end_index - start_index == 0 we need to add that exact charactger to substring.
    
    new_string.data = alloc(new_string.arena, new_string.count + 1); // +1 for null termination.
    // @TODO: think about null termination.
    mem_copy(new_string.data, original_string.data + start_index, new_string.count * sizeof(char));
    new_string.data[new_string.count] = '\0';
    return new_string;
}

inline i32 string_find_from(String string, String to_find, i32 start_index) {
    if (string.count <= start_index || to_find.count == 0 || to_find.count > string.count) {
        return -1;
    }

    // We'll fill data and count of this for_compare string for ourselves now, but that's actually could easily be done with 
    // copy_string if we would remove null-termination.
    String for_compare = {0};
    for_compare.count = to_find.count;
    
    for (i32 i = start_index; i < string.count - to_find.count + 1; i++) {
        for_compare.data = string.data + i;
        
        if (string_equal(to_find, for_compare)) {
            return i;
        }
    }
    
    return -1;
}
inline i32 string_find(String string, String to_find) {
    return string_find_from(string, to_find, 0);
}

inline i32 string_find_from_back(String string, String to_find) {       
    if (string.count < to_find.count || to_find.count == 0) return -1;
    
    String for_compare = {0};
    for_compare.count = to_find.count;
    
    for (i32 i = string.count - to_find.count; i >= 0; i--) {
        for_compare.data = string.data + i;
        if (string_equal(to_find, for_compare)) {
            return i;
        }
    }
    
    return -1;
}

inline bool string_contains(String s, String to_find) {
    return string_find(s, to_find) >= 0;
}

void builder_append_char(String_Builder *builder, char ch);
void builder_append_str(String_Builder *builder, const char *str);
void builder_append(String_Builder *builder, String string);
String make_string_from_builder(String_Builder *builder, Memory_Arena *arena);

String tstring(const char *text, ...) {
    String result_string = {.arena = temp};
    
    va_list args;
    va_start(args, text);
    i32 byte_count = vsnprintf(result_string.data, 0, text, args);
    result_string.data = alloc(result_string.arena, byte_count + 1);
    vsnprintf(result_string.data, byte_count + 1, text, args);
    va_end(args);
    
    result_string.count = byte_count;
    
    return result_string;
}

typedef struct Medium_Str{
    char data[MEDIUM_STR_LEN];  
} Medium_Str;

#define LONG_STR_LEN 2048

typedef struct Long_Str {
    char data[LONG_STR_LEN];  
} Long_Str;


char *c_str_builder(String_Builder *builder) {
    // Currently we null-terminate strings and builders by default, so returning just data.
    return builder->data;
}

void init_string_builder(String_Builder *builder, i32 capacity) {
    assert(!builder->data);
    assert(builder->capacity == 0);
    builder->data = alloc(builder->arena, capacity);
    builder->capacity = capacity;
    builder->count = 0;
}

// @TODO: I would like to get rid of null ternimation on strings, but while we working with Raylib that could be hard.
static inline void builder_grow_if_need(String_Builder *builder, int appended_count) {
    assert(builder->data && builder->capacity > 0);
    
    if (builder->count + appended_count + 1 > builder->capacity) {
        char *old_data = builder->data;
        
        while (builder->count + appended_count + 1 > builder->capacity) {
            builder->capacity *= 2;
        }
        
        builder->data = alloc(builder->arena, builder->capacity);
        mem_copy(builder->data, old_data, builder->count * sizeof(char));
        
        free_data_in_arena(builder->arena, old_data);
    }
}


static inline void builder_grow_if_need_string(String_Builder *builder, String appended_string) {
    builder_grow_if_need(builder, appended_string.count);
}

void builder_append_str(String_Builder *builder, const char *appended_str) {
    if (!builder->data) init_string_builder(builder, 2048);
    
    size_t len = str_len(appended_str);
    
    builder_grow_if_need(builder, len);    
    
    mem_copy(builder->data + builder->count, (void *)appended_str, len * sizeof(char));
    builder->count += len;
    builder->data[builder->count] = '\0';
}
void builder_append(String_Builder *builder, String appended_string) {
    if (!builder->data) init_string_builder(builder, 2048);
    
    builder_grow_if_need_string(builder, appended_string);    
    
    mem_copy(builder->data + builder->count, appended_string.data, appended_string.count * sizeof(char));
    builder->count += appended_string.count;
    builder->data[builder->count] = '\0';
}

void builder_append_char(String_Builder *builder, char ch) {
    if (!builder->data) init_string_builder(builder, 512);
    
    builder_grow_if_need(builder, 1);
    
    builder->data[builder->count] = ch;
    builder->count += 1;
    builder->data[builder->count] = '\0';
}

void builder_free(String_Builder *builder) {
    assert(builder->data);
    free_data_in_arena(builder->arena, builder->data);
}

inline String make_string_from_builder(String_Builder *builder, Memory_Arena *arena) {
    // @TODO: This works while we're using null termination.
    return string(arena, builder->data);
}

String_Builder make_string_builder(String string, Memory_Arena *arena) {
    String_Builder builder = {.arena = arena};
    init_string_builder(&builder, string.count);
    builder_append(&builder, string);
    return builder;
}

i32 string_find_digit(String s) {
    for (i32 i = 0; i < s.count; i++) {
        if (is_digit(s.data[i])) return i;
    }
    
    return -1;
}

#include "CArray.c"

DEFINE_ARRAY(Array_String, String, string_equal);

void split_string_to_existing(Array_String *splitted, String to_split, String separators) {
    if (separators.count <= 0) {    
        return;
    }
    
    String_array_clear(splitted);
    int ground_index = 0;
    
    for (int i = 0; i < to_split.count; i++) {
        for (int s = 0; s < separators.count; s++) {
            if (to_split.data[i] == separators.data[s]) {
                // That check exists for continuous separators.
                if (ground_index < i) {                
                    String_array_append(splitted, make_substring(to_split, ground_index, i - 1, splitted->arena)); // i - 1 because make_substring include end index and we don't want to add separator to string.
                }
                    
                ground_index = i + 1;
            }
        }
    }
    
    // At that point ground index should be equal to (last separator + 1) and we need to add last substring to array.
    // So if last separator was last symbol - ground_index would be equal to to_split.count.
    if (ground_index <= to_split.count - 1) {
        String_array_append(splitted, make_substring(to_split, ground_index, to_split.count - 1, splitted->arena));
    }
}

Array_String split_string(String to_split, String separators, Memory_Arena *arena) {
    Array_String result = {.arena = arena};     
    split_string_to_existing(&result, to_split, separators);    
    return result;
}

