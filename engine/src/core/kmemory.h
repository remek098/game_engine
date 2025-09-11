#pragma once

#include "defines.h"

enum memory_tags {
    MEMORY_TAG_UNKNOWN              = 0,
    MEMORY_TAG_ARRAY                = 1,
    MEMORY_TAG_DARRAY               = 2,
    MEMORY_TAG_DICT                 = 3,
    MEMORY_TAG_RING_QUEUE           = 4,
    MEMORY_TAG_BST                  = 5,
    MEMORY_TAG_STRING               = 6,
    MEMORY_TAG_APPLICATION          = 7,
    MEMORY_TAG_JOB                  = 8,
    MEMORY_TAG_TEXTURE              = 9,
    MEMORY_TAG_MATERIAL_INSTANCE    = 10,
    MEMORY_TAG_RENDERER             = 11,
    MEMORY_TAG_GAME                 = 12,
    MEMORY_TAG_TRANSFORM            = 13,
    MEMORY_TAG_ENTITY               = 14,
    MEMORY_TAG_ENTITY_NODE          = 15,
    MEMORY_TAG_SCENE                = 16,
    
    MEMORY_TAG_MAX_TAGS
};

// I guess I could just use namespace for it

// subsystem for managing memory allocations
class KAPI kmemory {
public:
    static void initialize();
    static void shutdown();

    // allocate will always return zero'ed out data with proper size allocated in memory
    static void* allocate(u64 size, memory_tags tag);
    static void free(void* block, u64 size, memory_tags tag);
    static void* zero(void* block, u64 size);
    static void* copy(void* dest, const void* source, u64 size);
    static void* set(void* dest, i32 value, u64 size);


    // not ideal, but that's not supposed to be called often, so no real worries about performance
    // Every time you call that, you also have to free the returned char buffer
    static char* get_memory_usage_string();
};