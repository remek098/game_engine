#pragma once
#include "defines.h"

typedef union vec2_u {
    f32 elements[2]; // array of x, y
    struct {
        // 1st element
        union {
            f32 x, r, s, u;
        };

        // 2nd element
        union {
            f32 y, g, t, v;
        };
        
    };
} vec2;


typedef struct vec3_u {
    union {
        // an array of x, y, z
        f32 elements[3];
        
        struct {
            // 1st element
            union {
                f32 x, r, s, u;
            };

            // 2nd element
            union {
                f32 y, g, t, v; 
            };

            // 3rd element
            union {
                f32 z, b, p, w;
            };
        };
    };
} vec3;


typedef union vec4_u {
#if defined(KUSE_SIMD)
    alignas(16) __m128 data;
#endif


    // an array of x, y, z, w
    alignas(16) f32 elements[4];
    
    union {
        struct {
            // 1st element
            union {
                f32 x, r, s;
            };

            // 2nd element
            union {
                f32 y, g, t; 
            };

            // 3rd element
            union {
                f32 z, b, p;
            };

            // 4th element
            union {
                f32 w, a, q;
            };
        };
    };
} vec4;


typedef vec4 quat;

typedef union mat4_u {
    alignas(16) f32 data[16];

#if defined(KUSE_SIMD)
    // used for SIMD operations
    alignas(16) vec4 rows[4];
#endif
} mat4;