#include "core/event.h"
#include "core/kmemory.h"
#include "containers/darray.h"

typedef struct registered_event {
    void* listener;
    PFN_on_event callback;
} registered_event;

// represents a single message code to which we can register multiple listeners
// e.g. you press a key, and you can have multiple things listening for that type of input
typedef struct event_code_entry {
   darray<registered_event> events; 
} event_code_entry;

// This should be more than enough codes... 2^14
#define MAX_MESSAGE_CODES 16384

// State structure. Holds all data related to events
typedef struct event_system_state {
    // Lookup table for event codes.
    event_code_entry registered[MAX_MESSAGE_CODES];
} event_system_state;


// Event system internal state
b8                  event_system::is_initialized = false;
event_system_state  event_system::state;

b8 event_system::initialize() {
    if(is_initialized == true) {
        return false;
    }
    is_initialized = false;
    kmemory::zero(&state, sizeof(state));

    is_initialized = true;
    return true;
}

void event_system::shutdown() {
    // free the events arrays. And objects pointed to should be destroyed on their own.
    for(u16 i = 0; i < MAX_MESSAGE_CODES; ++i) {
        if(state.registered[i].events.get_length() > 0) {
            // make sure all registered events are being destroyed (whole array's memory is freed)
            state.registered[i].events.destroy();
        }
    }
}

b8 event_system::register_event(u16 code, void* listener, PFN_on_event on_event) {
    if(is_initialized == false) {
        return false;
    }

    // if array is still not initialized, initialize it with default capacity
    if(state.registered[code].events.is_valid() == false) {
        state.registered[code].events.create();
    }

    // if darray just got initialized in previous if statement, length is 0 and we don't check for listeners
    u64 registered_count = state.registered[code].events.get_length();
    for(u64 i = 0; i < registered_count; ++i) {
        if(state.registered[code].events[i].listener == listener) {
            // TODO: warn
            // obviously don't register if it already is registered
            return false;
        }
    }

    // if at this point no duplicate was found, proceed with registration.
    registered_event event;
    event.listener = listener;
    event.callback = on_event;
    state.registered[code].events.push(event);

    return true;
}

b8 event_system::unregister_event(u16 code, void* listener, PFN_on_event on_event) {
    if(is_initialized == false) {
        return false;
    }

    // if we didn't find anything in the array for our code
    if(state.registered[code].events.is_valid() == false) {
        // TODO: warn
        return false;
    }

    u64 registered_count = state.registered[code].events.get_length();
    for(u64 i = 0; i < registered_count; ++i) {
        registered_event e = state.registered[code].events[i];
        if(e.listener == listener && e.callback == on_event) {
            // Found one, execute order 66
            registered_event popped_event;
            state.registered[i].events.pop_at(i, &popped_event);
            return true;
        }
    }

    // Didn't find it
    return false;
}

b8 event_system::fire_event(u16 code, void* sender, event_context context) {
    if(is_initialized == false) {
        return false;
    }

    // if nothing is registered for a code, boot out
    if(state.registered[code].events.is_valid() == false) {
        // TODO: warn
        return false;
    }

    u64 registered_count = state.registered[code].events.get_length();
    for(u64 i = 0; i < registered_count; ++i) {
        registered_event e = state.registered[code].events[i];
        if(e.callback(code, sender, e.listener, context)) {
            // message has been handled, do not send to other listeners
            return true;
        }
    }
    
    return false;
}