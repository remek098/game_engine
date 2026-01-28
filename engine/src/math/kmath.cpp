#include "kmath.h"

#include "platform/platform.h"

static b8 rand_seeded = false;

#include <math.h>

f32 kmath::ksin(f32 x) {
    return sinf(x);
}

f32 kmath::kcos(f32 x) {
    return cosf(x);
}

f32 kmath::ktan(f32 x) {
    return tanf(x);
}

f32 kmath::kacos(f32 x) {
    return acosf(x);
}

f32 kmath::ksqrt(f32 x) {
    return sqrtf(x);
}

f32 kmath::kabs(f32 x) {
    return fabsf(x);
}

i32 kmath::krandom() {
    if(!rand_seeded) {
        srand((u32)platform::get_absolute_time());
        rand_seeded = true;
    }
    return rand();
}

i32 kmath::krandom_in_range(i32 min, i32 max) {
    if (!rand_seeded) {
        srand((u32)platform::get_absolute_time());
        rand_seeded = true;
    }
    return (rand() % (max - min + 1)) + min;
}

f32 kmath::fkrandom() {
    return (f32)krandom() / (f32)RAND_MAX;
}

f32 kmath::fkrandom_in_range(f32 min, f32 max) {
    return min + ((float)krandom() / ((f32)RAND_MAX / (max - min)));
}
