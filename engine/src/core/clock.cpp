#include "clock.h"
#include "platform/platform.h"

clock::~clock() {
}

void clock::start() {
    this->start_time = platform::get_absolute_time();
    this->elapsed = 0;
}

void clock::stop() {
    this->start_time = 0;
}

void clock::update() {
    if(this->start_time != 0) {
        this->elapsed = platform::get_absolute_time() - this->start_time;
    } 
}
