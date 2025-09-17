#pragma once
#include "defines.h"

typedef struct event_context {
    // 128 bytes
    union 
    {
        i64 i64[2];
        u64 u64[2];
        f64 f64[2];
        
        i32 i32[4];
        u32 u32[4];
        f32 f32[4];

        i16 i16[8];
        u16 u16[8];

        i8 i8[16];
        u8 u8[16];

        char c[16];
    } data;

    const u16 get_key_code() const { return data.u16[0]; }
    
} event_context;

// should return true if handled
typedef b8 (*PFN_on_event)(u16 code, void* sender, void* listener_inst, event_context data);

struct event_system_state;

/**
 * @brief This event system is immediate. Will have to multi-thread this later on.
 * 
 * Will have to define priority of event messages later on too.
 * 
 */
class event_system {
public:
    static b8 initialize();
    static void shutdown();

    /**
     * @brief Register to listen for when events are sent with the provided code. 
     * 
     * Events with duplicate listener/callback combos will not be registered again and will cause
     * this to return false.
     * 
     * @param code The event code to listen for.
     * @param listener A pointer to listener instance. Can be 0/nullptr
     * @param on_event The callback function pointer to be invoked when the event code is fired.
     * @return true if the event is successfully registered, otherwise false
     */
    static KAPI b8 register_event(u16 code, void* listener, PFN_on_event on_event);

    /**
     * @brief Unregister from listening for when events are sent with the provided code.
     *
     *  If no matching registration is found, this funtion returns false.
     * 
     * @param code The event code to stop listening for.
     * @param listener A pointer to listener instance. Can be 0/nullptr
     * @param on_event The callback function pointer to be unregistered.
     * @return truue if the event is successfully unregistered; otherwise false.
     */
    static KAPI b8 unregister_event(u16 code, void* listener, PFN_on_event on_event);

    /**
     * @brief Fires an event to listeners of the given code. If an event handler returns
     * true, the event is considered handled and is not passed on to any more listeners.
     * 
     * @param code The event code to fire.
     * @param sender A pointer to the sender. Can be 0/nulltpr
     * @param context The event data
     * @return true if handled, otherwise false
     */
    static KAPI b8 fire_event(u16 code, void* sender, event_context context);

private:
    // event system internal state -> look more in cpp file
    static b8 is_initialized;
    static event_system_state state;
};


// system internal event codes. Application should use codes beyond 255.
enum system_event_codes {
    // shuts the application down on the next frame
    EVENT_CODE_APPLICATION_QUIT = 0x01,

    // keyboard key pressed.
    /* Context usage:
        u16 keycode = data.data.u16[0];
    */
    EVENT_CODE_KEY_PRESSED = 0x02,

    // keyboard key released.
    /* Context usage:
        u16 keycode = data.data.u16[0];
    */
   EVENT_CODE_KEY_RELEASED = 0x03,

   // mouse button pressed
   /*Context usage: 
        u16 button = data.data.u16[0];
   */
    EVENT_CODE_BUTTON_PRESSED = 0x04,
    
    // mouse button released
   /*Context usage: 
        u16 button = data.data.u16[0];
   */
    EVENT_CODE_BUTTON_RELEASED = 0x05,

    // mouse moved:
    /*Context usage:
        u16 x = data.data.u16[0];
        u16 y = data.data.u16[1];
    */
   EVENT_CODE_MOUSE_MOVED = 0x06,

   // mouse wheel
   /* Context usage: 
        u8 z_delta = data.data.u8[0];
   */
    EVENT_CODE_MOUSE_WHEEL = 0x07,

    // resized/resolution changed from the OS.
    /* Context usage:
        u16 width = data.data.u16[0];
        u16 height = data.data.u16[1];
    */
    EVENT_CODE_RESIZED = 0x08,

    MAX_EVENT_CODE = 0xFF
};