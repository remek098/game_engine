#include "application.h"

#include "game_types.h"

#include "core/logger.h"
#include "core/kmemory.h"
#include "core/input.h"

#include "renderer/renderer_frontend.h"

#include <../../testbed/src/game.h>


// #include "containers/darray.h"
// #include "containers/darray.cpp"

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
    input_system::initialize();

    // TODO: Remove this.
    KFATAL("Test message: %f", 3.14f);
    KERROR("Test message: %f", 3.14f);
    KWARN("Test message: %f", 3.14f);
    KINFO("Test message: %f", 3.14f);
    KDEBUG("Test message: %f", 3.14f);
    KTRACE("Test message: %f", 3.14f);

    // u32 arr[3] = {5, 6, 7};
    // darray<u32> vec(arr, 3);
    // vec.push(4);
    // vec.insert_at(2, 21);

    // KINFO("vec length is: %i", vec.get_length());
    
    // for(u32 i = 0; i < vec.get_length(); i++) {
    //     KINFO("vec[%i] = %i", i, vec[i]);
    // }

    // u32 buff;
    // vec.pop_at(2, &buff);

    // KINFO("vec length is: %i", vec.get_length());
    
    // for(u32 i = 0; i < vec.get_length(); i++) {
    //     KINFO("vec[%i] = %i", i, vec[i]);
    // }
    // vec.destroy();

    app_state.is_running = true;
    app_state.is_suspended = false;

    if(!event_system::initialize()) {
        KERROR("Event system failed initialization. Application cannot continue.");
        return false;
    }

    event_system::register_event(EVENT_CODE_APPLICATION_QUIT, 0, &application::app_on_event);
    event_system::register_event(EVENT_CODE_KEY_PRESSED, 0, &application::app_on_key);
    event_system::register_event(EVENT_CODE_KEY_RELEASED, 0, &application::app_on_key);
    event_system::register_event(EVENT_CODE_RESIZED, 0, &application::app_on_resized);
    

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

    // initialize renderer
    if(!renderer::initialize(game_inst->app_config.name, app_state.platform->plat_state)) {
        KFATAL("Failed to initialize renderer. Aborting application");
        return false;
    }


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
    app_state.clock.start();
    app_state.clock.update();
    app_state.last_time = app_state.clock.elapsed;

    [[maybe_unused]] f64 running_time = 0;
    [[maybe_unused]] u8 frame_count = 0;
    f64 target_frame_seconds = 1.0f / 60;

    KINFO(kmemory::get_memory_usage_string());

    while(app_state.is_running) {
        if(!app_state.platform->pump_messages()) {
            app_state.is_running = false;
        }

        if(!app_state.is_suspended) {

            // update clock and delta-time
            app_state.clock.update();
            f64 current_time = app_state.clock.elapsed;
            f64 delta = (current_time - app_state.last_time);
            f64 frame_start_time = platform::get_absolute_time();

            if(!app_state.game_inst->update(app_state.game_inst, (f32)delta)) {
                KFATAL("Game update failed, shutting down.");
                app_state.is_running = false;
                break;
            }
            // render routine
            if(!app_state.game_inst->render(app_state.game_inst, (f32)delta)) {
                KFATAL("Game render failed, shutting down.");
                app_state.is_running = false;
                break;
            }

            // TODO: refactor packet creation
            render_packet packet;
            packet.delta_time = delta;
            renderer::draw_frame(&packet);

            // figure out how long the frame took 
            f64 frame_end_time = platform::get_absolute_time();
            f64 frame_elapsed_time = frame_end_time - frame_start_time;
            running_time += frame_elapsed_time;
            f64 remaining_seconds  = target_frame_seconds - frame_elapsed_time;

            if(remaining_seconds > 0) {
                u64 remaining_ms = (remaining_seconds * 1000);

                // if there's time left, give it back to OS
                b8 limit_frames = false;
                if(remaining_ms > 0 && limit_frames) {
                    platform::sleep(remaining_ms - 1);
                }

                frame_count++;
            }

            // NOTE: Input update/state copying should always be handled after any input should be recorded;
            // i.e. before this line.
            // as a safety, input is the last thing to be updated before this frame ends.
            input_system::update(delta);

            // update last_time for next frame
            app_state.last_time = current_time;
        }

    }
    // application should not run anymore after exiting while loop -> sanity check
    app_state.is_running = false;

    // probably not needed but just to be explicit
    // unregister tracked by application events before shutting down event_system
    event_system::unregister_event(EVENT_CODE_APPLICATION_QUIT, 0, &application::app_on_event);
    event_system::unregister_event(EVENT_CODE_KEY_PRESSED, 0, &application::app_on_key);
    event_system::unregister_event(EVENT_CODE_KEY_RELEASED, 0, &application::app_on_key);
    event_system::unregister_event(EVENT_CODE_RESIZED, 0, &application::app_on_resized);

    // event_system::shutdown() automatically cleans up all events. Ofc we will have to unregister events
    // during runtime of application later on.
    event_system::shutdown();
    input_system::shutdown();

    renderer::shutdown();

    app_state.platform->shutdown();
    
    
    if(app_state.platform)
        delete app_state.platform;


    kmemory::free(app_state.game_inst->state, sizeof(game_state), MEMORY_TAG_GAME);
    // KINFO("From application::run:")
    // KINFO(kmemory::get_memory_usage_string());
    return true;
}

void application::app_get_framebuffer_size(u32* width, u32* height) {
    *width = app_state.width;
    *height = app_state.height;
}

b8 application::app_on_event(u16 code, void* sender, void* listener_inst, event_context context) {
    switch(code) {
        case EVENT_CODE_APPLICATION_QUIT: {
            KINFO("EVENT_CODE_APPLICATION_QUIT received, shutting down. \n");
            app_state.is_running = false;
            return true;
        }
    }

    return false;
}

b8 application::app_on_key(u16 code, void* sender, void* listener_inst, event_context context) {
    
    if(code == EVENT_CODE_KEY_PRESSED) {
        u16 key_code = context.get_key_code(); // context.data.u16[0];
        if(key_code == KEYBOARD_KEY_ESCAPE) {
            // NOTE: technically firing an event to itself, but there may be listeners
            event_context data = {};
            event_system::fire_event(EVENT_CODE_APPLICATION_QUIT, 0, data);

            // blocking anything else from processing this
            return true;
        }
        else if(key_code == KEYBOARD_KEY_A) {
            // example
            KDEBUG("Explicit - A was pressed!");
        }
        else {
            KDEBUG("'%c' key released in window", key_code);
        }
    }
    else if(code == EVENT_CODE_KEY_RELEASED) {
        u16 key_code = context.get_key_code();// context.data.u16[0]; // in input_system::process_key first u16 (of index 0) is set to a key_code
        if(key_code == KEYBOARD_KEY_B) {
            // example
            KDEBUG("Explicit - B was pressed!");
        }
        else {
            KDEBUG("'%c' key released in window", key_code);
        }
    }
    return false;
}

b8 application::app_on_resized(u16 code, void* sender, void* listener_inst, event_context context) {
    if(code == EVENT_CODE_RESIZED) {
        u16 width = context.data.u16[0];
        u16 height = context.data.u16[1];

        // check if diffrent. If so, trigger a resize event.
        if(width != app_state.width || height != app_state.height) {
            app_state.width = width;
            app_state.height = height;

            KDEBUG("Window resize occured: %i, %i", width, height);

            // handle minimalization of window
            if(width == 0 || height == 0) {
                KINFO("Window minimized, suspending application.");
                app_state.is_suspended = true;
                return true;
            }
            else {
                if(app_state.is_suspended) {
                    KINFO("Window restored, resuming application.");
                    app_state.is_suspended = false;
                }
                app_state.game_inst->on_resize(app_state.game_inst, width, height);
                renderer::on_resized(width, height);
            }
        }
    }

    // event purposefully not handled to allow other listeners to get this.
    return false;
}
