#include "darray.h"

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