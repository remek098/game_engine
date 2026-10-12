#include "kmemory.h"
#include "core/logger.h"
#include "platform/platform.h"

// TODO: Custom string library
#include <string.h>
#include <stdio.h>

// could move it to .h file and make a static private field for stats, but it's only relevant in this cpp file anyway and we 
// will have an access to kmemory::get_memory_usage_string() anyway to track our allocations in console environment
struct memory_stats {
    u64 total_allocated{0};
    u64 tagged_allocations[MEMORY_TAG_MAX_TAGS]{};
};

static const char* memory_tag_strings[MEMORY_TAG_MAX_TAGS] = {
    "UNKNOWN     ",     
    "ARRAY       ",
    "LINEAR_ALLOC",
    "DARRAY      ",
    "DICT        ",
    "RING_QUEUE  ",
    "BST         ",
    "STRING      ",
    "APPLICATION ",
    "JOB         ",
    "TEXTURE     ",
    "MAT_INSTANCE",
    "RENDERER    ",
    "GAME        ",
    "TRANSFORM   ",
    "ENTITY      ",
    "ENTITY_NODE ",
    "SCENE       "
};



typedef struct memory_system_state {
    struct memory_stats stats;

} memory_system_state;

static memory_system_state* state_ptr = nullptr; // = static_cast<memory_system_state*>(platform::allocate(sizeof(memory_system_state), false));

void kmemory::initialize() {
    //state_ptr = static_cast<memory_system_state*>(platform::allocate(sizeof(memory_system_state), false));
    if(!state_ptr) {
        state_ptr = static_cast<memory_system_state*>(platform::allocate(sizeof(memory_system_state), false));
        platform::zero_memory(&state_ptr->stats, sizeof(memory_stats));
    }
}

void kmemory::shutdown() {
    KINFO("From kmemory::shutdown():");
    KINFO(get_memory_usage_string());
    platform::free(state_ptr, false);

}


void* kmemory::allocate(u64 size, memory_tags tag) {
    if(!state_ptr) {
        kmemory::initialize();
    }
    

    if(tag == MEMORY_TAG_UNKNOWN) {
        KWARN("kmemory::allocate() called using MEMORY_TAG_UNKNOWN. Re-class this allocation.");
    }

    state_ptr->stats.total_allocated += size; // size in bytes
    state_ptr->stats.tagged_allocations[tag] += size;

    // TODO: Memory alignment
    void* block = platform::allocate(size, false);
    // KDEBUG("kmemory::allocate -> DARRAY allocation size: %i", stats.tagged_allocations[MEMORY_TAG_DARRAY]);
    platform::zero_memory(block, size);
    return block;
}


void kmemory::free(void* block, u64 size, memory_tags tag) {
    if(tag == MEMORY_TAG_UNKNOWN) {
        KWARN("kmemory::free() called using MEMORY_TAG_UNKNOWN. Re-class this allocation.");
    }

    
    // TODO: Memory alignment
    platform::free(block, false);
    // KDEBUG("kmemory::free -> DARRAY allocation size: %i", stats.tagged_allocations[MEMORY_TAG_DARRAY]);
    // KINFO(kmemory::get_memory_usage_string());
    state_ptr->stats.total_allocated -= size; // size in bytes
    state_ptr->stats.tagged_allocations[tag] -= size;
}


void* kmemory::zero(void* block, u64 size) {
    return platform::zero_memory(block, size);
}


void* kmemory::copy(void* dest, const void* source, u64 size) {
    return platform::copy_memory(dest, source, size);
}


void* kmemory::set(void* dest, i32 value, u64 size) {
    return platform::set_memory(dest, value, size);
}



char* kmemory::get_memory_usage_string() {
    const u64 gib = 1024 * 1024 * 1024;
    const u64 mib = 1024 * 1024;
    const u64 kib = 1024;

    char buffer[8000] = "System memory use (tagged): \n";
    u64 offset = strlen(buffer);
    for(u32 i = 0; i < MEMORY_TAG_MAX_TAGS; ++i) {
        char unit[4] = "XiB";
        float amount = 1.0f;
        if(state_ptr->stats.tagged_allocations[i] >= gib) {
            unit[0] = 'G';
            amount = state_ptr->stats.tagged_allocations[i] / (f32)gib;
        }
        else if(state_ptr->stats.tagged_allocations[i] >= mib) {
            unit[0] = 'M';
            amount = state_ptr->stats.tagged_allocations[i] / (f32)mib;
        }
        else if(state_ptr->stats.tagged_allocations[i] >= kib) {
            unit[0] = 'K';
            amount = state_ptr->stats.tagged_allocations[i] / (f32)kib;
        }
        else {
            unit[0] = 'B';
            unit[1] = 0; // NULL char
            amount = (f32)state_ptr->stats.tagged_allocations[i];
        }

        i32 length = snprintf(buffer + offset, 8000, "  %s: %.2f%s\n", memory_tag_strings[i], amount, unit);
        offset += length;
    }
    char* out_string = _strdup(buffer);
    return out_string;
}

