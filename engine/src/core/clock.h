#pragma once

#include "defines.h"

class KAPI kclock {
public:
    kclock() = default;
    ~kclock();
    // starts the clock. Resets elapsed time;
    void start();
    void stop();

    void update();
public:
    f64 start_time = 0.0;
    f64 elapsed = 0.0;
};