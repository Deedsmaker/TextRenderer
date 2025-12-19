#pragma once

#include <math.h>

#define PI 3.14159265359f

typedef struct Vector2_i32 { 
    i32 x, y;
} Vector2_i32;

i32 i32_clamp(i32 num, i32 min, i32 max) {
    if (num <= min) return min;
    if (num >= max) return max;
    return num;
}
