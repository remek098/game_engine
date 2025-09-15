#include "core/event.h"
#include "core/kmemory.h"

typedef struct registered_event {
    void* listener;
    PFN_on_event callback;
} registered_event;

// represents a single message code to which we can register multiple listeners
// e.g. you press a key, and you can have multiple things listening for that type of input
typedef struct event_code_entry {
   registered_event* events; 
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
