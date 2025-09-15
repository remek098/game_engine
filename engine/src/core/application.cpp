#include "application.h"
#include "game_types.h"
#include "core/logger.h"
#include "core/kmemory.h"


#include "containers/darray.h"
#include "containers/darray.cpp"

b8 application::is_initialized = false;
application_state application::app_state;

application:: application()
{
}

application::~application() {
    // delete app_state.platform;

}

b8 application::create(game* game_inst) {
    if(is_initialized) {
        KERROR("application_create called more than one time");
        return false;
    }

    app_state.game_inst = game_inst;
    app_state.width = game_inst->app_config.start_width;
    app_state.height = game_inst->app_config.start_height;
    

    // Initialize subsystems.
    initialize_logging();

    // TODO: Remove this.
    KFATAL("Test message: %f", 3.14f);
    KERROR("Test message: %f", 3.14f);
    KWARN("Test message: %f", 3.14f);
    KINFO("Test message: %f", 3.14f);
    KDEBUG("Test message: %f", 3.14f);
    KTRACE("Test message: %f", 3.14f);

    u32 arr[3] = {5, 6, 7};
    darray<u32> vec(arr, 3);
    vec.push(4);
    vec.insert_at(2, 21);

    KINFO("vec length is: %i", vec.get_length());
    
    for(u32 i = 0; i < vec.get_length(); i++) {
        KINFO("vec[%i] = %i", i, vec[i]);
    }
    // vec.destroy();

    app_state.is_running = true;
    app_state.is_suspended = false;

    // deleted at the end of application::run()
    platform* pt = new platform();
    app_state.platform = pt;
    if(!app_state.platform->startup(
        game_inst->app_config.name,
        game_inst->app_config.start_pos_x,
        game_inst->app_config.start_pos_y,
        game_inst->app_config.start_width,
        game_inst->app_config.start_height
    )) {    
        return false;
    }
    // delete pt; -> don't do it here, make sure that either in destructor or in application::run() that pointer is destroyed once. 

    // initialize the game
    if(!app_state.game_inst->initialize(app_state.game_inst)) {
        KFATAL("Game failed to initialize");
        return false;
    }

    app_state.game_inst->on_resize(app_state.game_inst, app_state.width, app_state.height);

    is_initialized = true;
    KINFO("Application initialized successfully");
    return true;
}

b8 application::run() {
    KINFO(kmemory::get_memory_usage_string());

    while(app_state.is_running) {
        if(!app_state.platform->pump_messages()) {
            app_state.is_running = false;
        }

        if(!app_state.is_suspended) {
            if(!app_state.game_inst->update(app_state.game_inst, (f32)0)) {
                KFATAL("Game update failed, shutting down.");
                app_state.is_running = false;
                break;
            }
            // render routine
            if(!app_state.game_inst->render(app_state.game_inst, (f32)0)) {
                KFATAL("Game render failed, shutting down.");
                app_state.is_running = false;
                break;
            }
        }

    }
    // application should not run anymore after exiting while loop -> sanity check
    app_state.is_running = false;

    app_state.platform->shutdown();
    
    if(app_state.platform)
        delete app_state.platform;

    
    return true;
}