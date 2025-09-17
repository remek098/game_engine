#pragma once
#include "defines.h"

#include "core/asserts.h"
#include "core/kmemory.h"

#include <memory>

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
    

    darray(u64 initial_capacity) {
        create(initial_capacity);
    }


    darray(const darray<T>& other) 
       : capacity(other.capacity), length(other.length), stride(other.stride)
    {
        if(capacity > 0) {
            // for now not aligned by allocate function itself
            array = static_cast<T*>(kmemory::allocate(capacity * stride, MEMORY_TAG_DARRAY));
            KASSERT_MSG(array, "Failed to allocate memory in darray copy constructor");
            kmemory::copy(array, other.array, length * stride);
        }
    }
    

    darray(const T* arr, u64 arr_length)  {
        destroy();
        stride = sizeof(T);
        length = arr_length;
        capacity = length;
        array = static_cast<T*>(kmemory::allocate(capacity*stride, MEMORY_TAG_DARRAY));
        KASSERT_MSG(array, "Failed to allocate memory.");
        kmemory::copy(array, arr, length*stride);
    }


    darray<T>& operator=(const darray<T>& other) {
        if(this != &other) {
            destroy();
            capacity = other.capacity;
            length = other.length;
            stride = other.stride;
            if(other.array && capacity > 0) {
                array = static_cast<T*>(kmemory::allocate(capacity * stride, MEMORY_TAG_DARRAY));
                KASSERT_MSG(array, "Failed to allocate memory in operator=");
                kmemory::copy(array, other.array, length * stride);
            }
        }

        return *this;
    }
    
    
    ~darray() {
       destroy();
    }


    T& operator[](u64 index) {
        KASSERT_MSG(index < length, "Index bigger than length");
        return array[index];
    }

    // creates and allocates completely new empty darray 
    T* create(u64 initial_capacity = DARRAY_DEFAULT_CAPACITY) {
        destroy(); // ensure that it has a clean state
        // making sure it has at least one
        capacity = (initial_capacity == 0) ? DARRAY_DEFAULT_CAPACITY : initial_capacity;
        length = 0;
        stride = sizeof(T);
        if(capacity > 0) {
            array = static_cast<T*>(kmemory::allocate(capacity * stride, MEMORY_TAG_DARRAY));
            KASSERT_MSG(array, "Failed to initialize memory in darray::create");
        }

        return array;
    }


    void destroy() {
        if(array) {
            kmemory::free(array, capacity * stride, MEMORY_TAG_DARRAY);
            array = nullptr;
        }
        capacity = 0;
        length = 0;
        stride = 0;
    }


    b8 is_valid() const {
        return array != nullptr && length > 0 && capacity > 0;
    }


// Operations
    // uses our enum 
    u64 field_get(u64 field) {
        switch (field) {
            case DARRAY_CAPACITY: return capacity;
            case DARRAY_LENGTH:   return length;
            case DARRAY_STRIDE:   return stride;
            default:              return 0;
        }
    }


    void field_set(u64 field, u64 value) {
        switch (field) {
            case DARRAY_CAPACITY: capacity = value; break;
            case DARRAY_LENGTH:   length = value; break;
            case DARRAY_STRIDE:   stride = value; break;
            default: break;
        }
    }


    T* resize()  {
        // KASSERT_MSG(array, "Cannot resize null array.");

        // if (stride != sizeof(T)) {
        //     stride = sizeof(T);  // or KASSERT_MSG here
        // }


        u64 new_capacity = capacity == 0 ? 1 : capacity * DARRAY_RESIZE_FACTOR;
        T* new_array = static_cast<T*>(kmemory::allocate(new_capacity * stride, MEMORY_TAG_DARRAY));
        KASSERT_MSG(new_array, "Failed to allocate memory during darray::resize.");

        if(array) {
            kmemory::copy(new_array, array, length * stride);
            kmemory::free(array, length * stride, MEMORY_TAG_DARRAY);
        }

        // basically old length and stride ofc remains, everything else changes
        array = new_array;
        capacity = new_capacity;

        return array;
    }


    void clear() {
        field_set(DARRAY_LENGTH, 0);
    }
    
    T* push(const T& value) {
        if(length >= capacity)
            resize();

        // just copy the value to the end of our darray
        array[length] = value;
        // basically return value and make sure length is increased by 1 to indicate new element has been added
        return &array[length++];
    }


    void pop(T* dest) {
        KASSERT_MSG(length > 0, "Cannot pop from empty array");
        if(dest) *dest = array[length - 1];
        --length;
    }
    

    void pop_at(u64 index, T* dest) {
        KASSERT_MSG(index < length, "Index out of bounds");
        if(dest)
            *dest = array[index];

        // make sure memory is continuous
        if (index != length - 1) {
            memmove(&array[index], &array[index + 1], (length - index - 1) * stride);
        }
        --length;
    }


    T* insert_at(u64 index, const T& other) {
        if (length >= capacity) 
            resize();
        
        if (index < length) {
            memmove(&array[index + 1], &array[index], (length - index) * stride);
        }
        array[index] = other;
        ++length;
        return &array[index];
    }

    // get fields
    u64 get_capacity() { return capacity; }
    u64 get_length() { return length; }

public:
    T* array = nullptr;
    u64 capacity = 0;
    u64 length = 0;
    u64 stride = 0; // set to sizeof(T) on initialization of darray in create()
};