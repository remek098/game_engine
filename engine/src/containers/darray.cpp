#include "darray.h"
#include "core/logger.h"

void* _darray_create(u64 length, u64 stride) {
    u64 header_size = DARRAY_FIELD_LENGTH * sizeof(u64);
    u64 array_size = length * stride;
    u64* new_array = reinterpret_cast<u64*>(kmemory::allocate(header_size + array_size, MEMORY_TAG_DARRAY));
    kmemory::set(new_array, 0, header_size + array_size);
    new_array[DARRAY_CAPACITY] = length;
    new_array[DARRAY_LENGTH] = 0;
    new_array[DARRAY_STRIDE] = stride;
    KINFO("Allocating %i memory", header_size + array_size);
    return (void*)(new_array + DARRAY_FIELD_LENGTH);
}

void _darray_destroy(void* array) {
    u64* header = (u64*)array - DARRAY_FIELD_LENGTH;
    u64 header_size = DARRAY_FIELD_LENGTH * sizeof(u64);
    u64 total_size = header_size + header[DARRAY_CAPACITY] * header[DARRAY_STRIDE];
    KINFO("Destroying %i memory", total_size);
    kmemory::free(header, total_size, MEMORY_TAG_DARRAY);
}

u64 _darray_field_get(void* array, u64 field) {
    u64* header = (u64*)array - DARRAY_FIELD_LENGTH;
    return header[field];
}

void _darray_field_set(void* array, u64 field, u64 value) {
    u64* header = (u64*)array - DARRAY_FIELD_LENGTH;
    header[field] = value;
}

void* _darray_resize(void* array) {
    u64 length = darray_length(array);
    u64 stride = darray_stride(array);
    void* temp = _darray_create(
        (DARRAY_RESIZE_FACTOR * darray_capacity(array)),
         stride
    );
    kmemory::copy(temp, array, length * stride);

    _darray_field_set(temp, DARRAY_LENGTH, length);
    _darray_destroy(array);
    return temp;
}

void* _darray_push(void* array, const void* value_ptr) {
    u64 length = darray_length(array);
    u64 stride = darray_stride(array);

    if(length >= darray_capacity(array)) {
        array = _darray_resize(array);
    }

    u64 addr = (u64)array;
    addr += (length * stride); // basically go to the end of the array
    kmemory::copy((void*)addr, value_ptr, stride); // copy the value to that new address in memory
    _darray_field_set(array, DARRAY_LENGTH, length + 1);
    return array;
}

void _darray_pop(void* array, void* dest) {
    u64 length = darray_length(array);
    u64 stride = darray_stride(array);
    u64 addr = (u64)array;
    addr += ((length - 1) * stride); // get the address from the end of arrray
    kmemory::copy(dest, (void*)addr, stride); // copy that value to our dest that we return 
    _darray_field_set(array, DARRAY_LENGTH, length - 1);

}

void* _darray_pop_at(void* array, u64 index, void* dest) {
    u64 length = darray_length(array);
    u64 stride = darray_stride(array);
    if(index >= length) {
        KERROR("Index outside bounds of the array! Length %i, index %i, ", length, index);
        return array;
    }

    u64 addr = (u64)array;
    kmemory::copy(dest, (void*)(addr + (index * stride)), stride);

    // if not on the last element, snip out the entry and copy the rest inward.
    if(index != length - 1) {
        kmemory::copy(
            (void*)(addr + (index  * stride)),
            (void*)(addr + ((index + 1) * stride)),
            stride * (length - index)
        );
    }

    _darray_field_set(array, DARRAY_LENGTH, length - 1);
    return array;
}

void* _darray_insert_at(void* array, u64 index, void* value_ptr) {
    u64 length = darray_length(array);
    u64 stride = darray_stride(array);
    if(index >= length) {
        KERROR("Index outside bounds of the array! Length %i, index %i, ", length, index);
        return array;
    }
    if(length >= darray_capacity(array)) {
        array = _darray_resize(array);
    }

    u64 addr = (u64)array;

    // if not the last element, copy the rest outward
    if(index != length - 1) {
        kmemory::copy(
            (void*)(addr + ((index + 1) * stride)),
            (void*)(addr + (index * stride)),
            stride * (length - index)
        );
    }

    // set the value at the index
    kmemory::copy((void*)(addr + (index * stride)), value_ptr, stride);
    
    _darray_field_set(array, DARRAY_LENGTH, length + 1);
    return array;

}


// template<typename T>
// darray<T>::darray(u64 initial_capacity) {
//     create(initial_capacity);
// }

// template<typename T>
// darray<T>::darray(const darray<T>& other)
//     : capacity(other.capacity), length(other.length), stride(other.stride)
// {
//     if(capacity > 0) {
//         // for now not aligned by allocate function itself
//         array = static_cast<T*>(kmemory::allocate(capacity * stride, MEMORY_TAG_DARRAY));
//         KASSERT_MSG(array, "Failed to allocate memory in darray copy constructor");
//         kmemory::copy(array, other.array, length * stride);
//     }
// }

// template<typename T>
// darray<T>::darray(const T* arr, u64 arr_length) {
//     destroy();
//     stride = sizeof(T);
//     length = arr_length;
//     capacity = length;
//     array = static_cast<T*>(kmemory::allocate(capacity*stride, MEMORY_TAG_DARRAY));
//     KASSERT_MSG(array, "Failed to allocate memory.");
//     kmemory::copy(array, arr, length*stride);
// }

// template<typename T>
// darray<T>& darray<T>::operator=(const darray<T>& other) {
//     if(this != &other) {
//         destroy();
//         capacity = other.capacity;
//         length = other.length;
//         stride = other.stride;
//         if(other.array && capacity > 0) {
//             array = static_cast<T*>(kmemory::allocate(capacity * stride, MEMORY_TAG_DARRAY));
//             KASSERT_MSG(array, "Failed to allocate memory in operator=");
//             kmemory::copy(array, other.array, length * stride);
//         }
//     }

//     return *this;
//}

// template<typename T>
// darray<T>::~darray() {
//     destroy();
// }

// template<typename T>
// T& darray<T>::operator[](u64 index) {
//     KASSERT_MSG(index < length, "Index bigger than length");
//     return array[index];
// }

// template<typename T>
// T* darray<T>::create(u64 initial_capacity) {
//     destroy(); // ensure that it has a clean state
//     // making sure it has at least one
//     capacity = (initial_capacity == 0) ? DARRAY_DEFAULT_CAPACITY : initial_capacity;
//     length = 0;
//     stride = sizeof(T);
//     if(capacity > 0) {
//         array = static_cast<T*>(kmemory::allocate(capacity * stride, MEMORY_TAG_DARRAY));
//         KASSERT_MSG(array, "Failed to initialize memory in darray::create");
//     }

//     return array;
// }

// template <typename T>
// void darray<T>::destroy() {
//     if(array) {
//         kmemory::free(array, length * stride, MEMORY_TAG_DARRAY);
//         array = nullptr;
//     }
//     capacity = 0;
//     length = 0;
//     stride = 0;
// }

// template<typename T>
// b8 darray<T>::is_valid() const {
//     return array != nullptr && capacity > 0;
// }

// template<typename T>
// u64 darray<T>::field_get(u64 field) {
//     switch (field) {
//         case DARRAY_CAPACITY: return capacity;
//         case DARRAY_LENGTH:   return length;
//         case DARRAY_STRIDE:   return stride;
//         default:              return 0;
//     }
// }

// template<typename T>
// void darray<T>::field_set(u64 field, u64 value) {
//     switch (field) {
//         case DARRAY_CAPACITY: capacity = value; break;
//         case DARRAY_LENGTH:   length = value; break;
//         case DARRAY_STRIDE:   stride = value; break;
//         default: break;
//     }
// }

// template<typename T>
// T* darray<T>::resize() {
//     // KASSERT_MSG(array, "Cannot resize null array.");

//     // if (stride != sizeof(T)) {
//     //     stride = sizeof(T);  // or KASSERT_MSG here
//     // }


//     u64 new_capacity = capacity == 0 ? 1 : capacity * DARRAY_RESIZE_FACTOR;
//     T* new_array = static_cast<T*>(kmemory::allocate(new_capacity * stride, MEMORY_TAG_DARRAY));
//     KASSERT_MSG(new_array, "Failed to allocate memory during darray::resize.");

//     if(array) {
//         kmemory::copy(new_array, array, length * stride);
//         kmemory::free(array, length * stride, MEMORY_TAG_DARRAY);
//     }

//     // basically old length and stride ofc remains, everything else changes
//     array = new_array;
//     capacity = new_capacity;

//     return array;
// }

// template<typename T>
// void darray<T>::clear() {
//     field_set(DARRAY_LENGTH, 0);
// }

// template<typename T>
// T* darray<T>:: push(const T& value) {
//     if(length >= capacity)
//         resize();

//     // just copy the value to the end of our darray
//     array[length] = value;
//     // basically return value and make sure length is increased by 1 to indicate new element has been added
//     return &array[length++];
// }

// template<typename T>
// void darray<T>::pop(T* dest) {
//     KASSERT_MSG(length > 0, "Cannot pop from empty array");
//     if(dest) *dest = array[length - 1];
//     --length;
// }

// template<typename T>
// void darray<T>::pop_at(u64 index, T* dest) {
//     KASSERT_MSG(index < length, "Index out of bounds");
//     if(dest)
//         *dest = array[index];

//     // make sure memory is continuous
//     if (index != length - 1) {
//         memmove(&array[index], &array[index + 1], (length - index - 1) * stride);
//     }
//     --length;
// }

// template<typename T>
// T* darray<T>::insert_at(u64 index, const T& other) {
//     if (length >= capacity) 
//         resize();
    
//     if (index < length) {
//         memmove(&array[index + 1], &array[index], (length - index) * stride);
//     }
//     array[index] = other;
//     ++length;
//     return &array[index];
// }