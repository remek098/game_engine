#pragma once
#include "defines.h"
#include "platform/platform.h"
#include "core/event.h"
#include "core/clock.h"
// #include "game_types.h"

struct game;

typedef struct application_state {
    game* game_inst;
    b8 is_running;
    b8 is_suspended;
    platform* platform;
    i16 width;
    i16 height;
    clock clock;
    f64 last_time;
} application_state;


typedef struct application_config {
    i16 start_pos_x;
    i16 start_pos_y;
    i16 start_width;
    i16 start_height;
    const char*  name;
} application_config;


// singleton -> we only want 1 instance of that running
class KAPI application {
public:
    application();
    ~application();

    // false by default, true only once we initialize our application
    static b8 is_initialized;
    static application_state app_state;
    // static application_config app_config;
    
    // static b8 create(i16 x, i16 y, i16 width, i16 height, const std::string& app_name);
    static b8 create(struct game* game_inst);
    static b8 run();
private:
    // application's event handlers
    static b8 app_on_event(u16 code, void* sender, void* listener_inst, event_context context);
    static b8 app_on_key(u16 code, void* sender, void* listener_inst, event_context context);

};