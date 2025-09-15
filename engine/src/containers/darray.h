#pragma once
#include "defines.h"

#include "core/asserts.h"
#include "core/kmemory.h"

#define DARRAY_DEFAULT_CAPACITY 1
#define DARRAY_RESIZE_FACTOR 2


enum {
    DARRAY_CAPACITY,
    DARRAY_LENGTH,
    DARRAY_STRIDE,
    DARRAY_FIELD_LENGTH
};

/*
Memory layout:
void* elements -> actual data we want to store inside dynamic array
u64 capacity -> number of elements that can be held
u64 length -> number of elements currently contained
u64 stride -> size of each element in bytes
*/

template<typename T>
class KAPI darray {
public:

// constructor based and manual option of managing object are available
    darray() = default;
    
    darray(u64 initial_capacity);
    // {
    //     create(initial_capacity);
    // }

    darray(const darray<T>& other);
    darray(const T* arr, u64 arr_length)  {
        destroy();
        stride = sizeof(T);
        length = arr_length;
        capacity = length;
        array = static_cast<T*>(kmemory::allocate(capacity*stride, MEMORY_TAG_DARRAY));
        KASSERT_MSG(array, "Failed to allocate memory.");
        kmemory::copy(array, arr, length*stride);
    }


    darray<T>& operator=(const darray<T>& other);
    
    ~darray() {
       destroy();
    }

    T& operator[](u64 index) {
        KASSERT_MSG(index < length, "Index bigger than length");
        return array[index];
    }

    T* create(u64 initial_capacity);

    void destroy() {
        if(array) {
            kmemory::free(array, capacity * stride, MEMORY_TAG_DARRAY);
            array = nullptr;
        }
        capacity = 0;
        length = 0;
        stride = 0;
    }

    b8 is_valid() const;

// Operations
    // uses our enum 
    u64 field_get(u64 field);
    void field_set(u64 field, u64 value);

    T* resize();
    void clear();
    
    T* push(const T& value);
    void pop(T* dest);
    
    void pop_at(u64 index, T* dest);
    T* insert_at(u64 index, const T& other);

    // get fields
    u64 get_capacity() { return capacity; }
    u64 get_length() { return length; }

public:
    T* array = nullptr;
    u64 capacity = 0;
    u64 length = 0;
    u64 stride = 0; // set to sizeof(T) on initialization of darray in create()
};