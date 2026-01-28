#pragma once

#include <core/logger.h>
#include <math/kmath.h>

#define expect_should_be(expected, actual)                                                                      \
    if(actual != expected) {                                                                                    \
        KERROR("--> Expected %lld, but got: %lld. File %s:%d.", expected, actual, __FILE__, __LINE__);          \
        return false;                                                                                           \
    }

#define expect_should_not_be(expected, actual)                                                                          \
    if(actual == expected) {                                                                                            \
        KERROR("--> Expected %d != %d, but they are equal. File: %s:%d.", expected, actual, __FILE__, __LINE__);         \
        return false;                                                                                                   \
    }

// might want to give a tolerance of K_FLOAT_EPSILON
#define expect_float_to_be(expected, actual)                                                                    \
    if(kmath::kabs(expected - actual) > 0.001f) {                                                               \
        KERROR("--> Expected %f, but got: %f. File: %s:%d.", expected, actual, __FILE__, __LINE__);             \
        return false;                                                                                           \
    }

#define expect_to_be_true(actual)                                                                               \
    if(actual != true) {                                                                                        \
        KERROR("--> Expected true, but got: false. File: %s:%d.", __FILE__, __LINE__);                          \
        return false;                                                                                           \
    }

#define expect_to_be_false(actual)                                                                               \
    if(actual != false) {                                                                                        \
        KERROR("--> Expected false, but got: true. File: %s:%d.", __FILE__, __LINE__);                          \
        return false;                                                                                           \
    }
