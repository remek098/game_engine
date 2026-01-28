#include "kstring.h"
#include "core/kmemory.h"

#include <cstring>

kstring::kstring(const char* str) {
    length_ = strlen(str);
    // data = static_cast<char*>(kmemory::allocate(length_ + 1, MEMORY_TAG_STRING));
    data = reinterpret_cast<char*>(kmemory::allocate(length_ + 1, MEMORY_TAG_STRING));
    kmemory::copy(data, str, length_ + 1); // copying +1 of length to ensure the null-terminator '\0' is included inside our string
}


kstring::kstring(const kstring& other) {
    length_ = other.length_;
    // data = static_cast<char*>(kmemory::allocate(length_ + 1, MEMORY_TAG_STRING));
    data = reinterpret_cast<char*>(kmemory::allocate(length_ + 1, MEMORY_TAG_STRING));
    kmemory::copy(data, other.data, length_ + 1);
}


kstring::~kstring() {
    kmemory::free(data, length_ + 1, MEMORY_TAG_STRING);
}


kstring& kstring::operator=(const kstring& other) {
    if(this != &other) {
        kmemory::free(data, length_ + 1, MEMORY_TAG_STRING);
        length_ = other.length_;
        // data = static_cast<char*>(kmemory::allocate(length_ + 1, MEMORY_TAG_STRING));
        data = reinterpret_cast<char*>(kmemory::allocate(length_ + 1, MEMORY_TAG_STRING));
        kmemory::copy(data, other.data, length_ + 1);
    }
    return *this;
}


kstring& kstring::operator=(const char* str) {
    // handle null input
    if(!str) {
        str = "";
    }

    u64 new_length = strlen(str);
    kmemory::free(data, length_ + 1, MEMORY_TAG_STRING);

    // data = static_cast<char*>(kmemory::allocate(new_length + 1, MEMORY_TAG_STRING));
    data = reinterpret_cast<char*>(kmemory::allocate(new_length + 1, MEMORY_TAG_STRING));
    kmemory::copy(data, str, new_length + 1);
    length_ = new_length;
    return *this;
}


kstring::kstring(kstring&& other) noexcept {
    data = other.data;
    length_ = other.length_;

    // Null out the source
    other.data = nullptr;
    other.length_ = 0;
}

kstring& kstring::operator=(kstring&& other) noexcept {
    if (this != &other) {
        // Free existing data
        kmemory::free(data, length_ + 1, MEMORY_TAG_STRING);

        // Steal other's data
        data = other.data;
        length_ = other.length_;

        // Null out source
        other.data = nullptr;
        other.length_ = 0;
    }
    return *this;
}


size_t kstring::length() const {
    return length_;
}


kstring kstring::duplicate() const {
    return kstring(data); // invoke copy constructor
}

// I guess we could also add && length_ == other.length to just be pedantic but we care about char* themselves.
bool kstring::equals(const kstring& other) const {
    return strcmp(data, other.data) == 0;
}

bool kstring::are_equal(const char* str1, const char* str2) {
    return strcmp(str1, str2) == 0;
}

const char* kstring::c_str() const {
    return data;
}

i32 kstring::format(char* dest, const char* format, ...) {
    if (dest) {
        __builtin_va_list arg_ptr; // might have to do va_list
        va_start(arg_ptr, format);
        i32 written = string_format_v(dest, format, arg_ptr);
        va_end(arg_ptr);
        return written;
    }
    return -1;
}

i32 kstring::string_format_v(char* dest, const char* format, va_list args) {
    if (dest) {
        // Big, but can fit on the stack.
        char buffer[32000];

        va_list args_copy;
        va_copy(args_copy, args);
        i32 written = vsnprintf(buffer, 32000, format, args_copy);
        va_end(args_copy);

        buffer[written] = 0;
        kmemory::copy(dest, buffer, written + 1);

        return written;
    }
    return -1;
}