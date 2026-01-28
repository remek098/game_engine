#include "clock.h"
#include "platform/platform.h"

kclock::~kclock() {
}

void kclock::start() {
    this->start_time = platform::get_absolute_time();
    this->elapsed = 0;
}

void kclock::stop() {
    this->start_time = 0;
}

void kclock::update() {
    if(this->start_time != 0) {
        this->elapsed = platform::get_absolute_time() - this->start_time;
    } 
}
