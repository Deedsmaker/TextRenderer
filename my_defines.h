#pragma once

#include <stdint.h>

typedef float  f32;
typedef double f64;
//#define _CRT_SECURE_NO_WARNINGS

#define global_variable static
#define local_persist   static
#define internal        static

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef u32 b32;

#define true 1
#define false 0

#define cast(type) type

#define Kilobytes(x)   ((size_t) (x) << 10)
#define Megabytes(x)   ((size_t) (x) << 20)

//#define For(container) for (auto it = (container).begin(); it != (container).end(); ++it)