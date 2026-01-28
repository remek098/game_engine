#include "input.h"

#include "core/event.h"
#include "core/kmemory.h"
#include "core/logger.h"

b8 input_system::initialized = false;
input_state input_system::state = {};

void input_system::initialize() {
    // being pedantic, static memory is zero'ed out by default
    kmemory::zero(&state, sizeof(input_state));
    initialized = true;
    KINFO("Input subsystem initalized");
}

void input_system::shutdown() {
    // TODO: Add shutdown routines when needed.
    initialized = false;
}

void input_system::update(f64 delta_time) {
    if(!initialized) {
        return;
    }

    // copy current states to previous states.
    kmemory::copy(&state.keyboard_previous_state, &state.keyboard_current_state, sizeof(keyboard_state));
    kmemory::copy(&state.mouse_previous_state, &state.mouse_current_state, sizeof(mouse_state));
}

void input_system::process_key(keyboard_keys key, b8 pressed) {
    if(key == KEYBOARD_KEY_LALT && pressed) {
        KINFO("Left alt pressed.");
    }
    else if(key == KEYBOARD_KEY_RALT && pressed) {
        KINFO("Right alt pressed.");
    }
    else if(key == KEYBOARD_KEY_LSHIFT && pressed) {
        KINFO("Left shift pressed.");
    }
    else if(key == KEYBOARD_KEY_RSHIFT && pressed) {
        KINFO("Right shift pressed.");
    }
    else if(key == KEYBOARD_KEY_LCONTROL && pressed) {
        KINFO("Left control pressed.");
    }
    else if(key == KEYBOARD_KEY_RCONTROL && pressed) {
        KINFO("Right control pressed.");
    }


    // only handle if state had changed
    if(state.keyboard_current_state.keys[key] != pressed) {
        // update internal state
        state.keyboard_current_state.keys[key] = pressed;

        // fire off an event for immidiate processing
        event_context context;
        context.data.u16[0] = key;
        event_system::fire_event(pressed ? EVENT_CODE_KEY_PRESSED : EVENT_CODE_KEY_RELEASED, nullptr, context);
    }
}

void input_system::process_button(mouse_buttons button, b8 pressed) {
    if(state.mouse_current_state.buttons[button] != pressed) {
        // update internal state
        state.mouse_current_state.buttons[button] = pressed;

        // fire the event
        event_context context;
        context.data.u16[0] = button;
        event_system::fire_event(pressed ? EVENT_CODE_BUTTON_PRESSED : EVENT_CODE_BUTTON_RELEASED, nullptr, context);
    }
}

void input_system:: process_mouse_move(i16 x, i16 y) {
    // only process if actually diffrent
    if(state.mouse_current_state.x != x || state.mouse_current_state.y != y) {
        // NOTE: Enable this if debugging
        // KDEBUG("Mouse pos: %i, %i!", x, y);

        // update internal state
        state.mouse_current_state.x = x;
        state.mouse_current_state.y = y;

        // fire the event
        event_context context;
        context.data.u16[0] = x;
        context.data.u16[1] = y;
        event_system::fire_event(EVENT_CODE_MOUSE_MOVED, 0, context);
    }
}

void input_system::process_mouse_wheel(i8 z_delta) {
    // NOTE: No internal state to update

    // fire the event
    event_context context;
    context.data.u8[0] = z_delta;
    event_system::fire_event(EVENT_CODE_MOUSE_WHEEL, 0, context);
}

b8 input_system::is_key_down(keyboard_keys key) {
    if(!initialized)
        return false;

    return state.keyboard_current_state.keys[key] == true;
}

b8 input_system::is_key_up(keyboard_keys key) {
    if(!initialized)
        return true;

    return state.keyboard_current_state.keys[key] == false;
}

b8 input_system::was_key_down(keyboard_keys key) {
    if(!initialized)
        return false;

    return state.keyboard_previous_state.keys[key] == true;
}

b8 input_system::was_key_up(keyboard_keys key) {
    if(!initialized)
        return true;

    return state.keyboard_previous_state.keys[key] == false;
}

b8 input_system::is_mouse_button_down(mouse_buttons button) {
    if(!initialized)
        return false;

    return state.mouse_current_state.buttons[button] == true;
}

b8 input_system::is_mouse_button_up(mouse_buttons button) {
    if(!initialized)
        return true;

    return state.mouse_current_state.buttons[button] == false;
}

b8 input_system::was_mouse_button_down(mouse_buttons button) {
    if(!initialized)
        return false;

    return state.mouse_previous_state.buttons[button] == true;
}

b8 input_system::was_mouse_button_up(mouse_buttons button) {
    if(!initialized)
        return true;

    return state.mouse_previous_state.buttons[button] == false;
}

void input_system::get_mouse_position(i32* x, i32* y) {
    if(!initialized) {
        *x = 0;
        *y = 0;
        return;
    }
    *x = state.mouse_current_state.x;
    *y = state.mouse_current_state.y;
}

void input_system::get_previous_mouse_position(i32* x, i32* y) {
    if(!initialized) {
        *x = 0;
        *y = 0;
        return;
    }
    *x = state.mouse_previous_state.x;
    *y = state.mouse_previous_state.y;
}