#pragma once

#include "defines.h"

enum memory_tags {
    MEMORY_TAG_UNKNOWN              = 0,
    MEMORY_TAG_ARRAY                ,
    MEMORY_TAG_LINEAR_ALLOCATOR     ,
    MEMORY_TAG_DARRAY               ,
    MEMORY_TAG_DICT                 ,
    MEMORY_TAG_RING_QUEUE           ,
    MEMORY_TAG_BST                  ,
    MEMORY_TAG_STRING               ,
    MEMORY_TAG_APPLICATION          ,
    MEMORY_TAG_JOB                  ,
    MEMORY_TAG_TEXTURE              ,
    MEMORY_TAG_MATERIAL_INSTANCE    ,
    MEMORY_TAG_RENDERER             ,
    MEMORY_TAG_GAME                 ,
    MEMORY_TAG_TRANSFORM            ,
    MEMORY_TAG_ENTITY               ,
    MEMORY_TAG_ENTITY_NODE          ,
    MEMORY_TAG_SCENE                ,
    
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