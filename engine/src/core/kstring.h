#pragma once

#include "defines.h"

class KAPI kstring {
public:
    kstring(const char* str = "");
    kstring(const kstring& other);
    ~kstring();

    kstring& operator=(const kstring& other);
    kstring& operator=(const char* str);

    // move constructor and move assignment semantic
    kstring(kstring&& other) noexcept;
    kstring& operator=(kstring&& other) noexcept;


    size_t length() const;
    kstring duplicate() const;

    bool equals(const kstring& other) const;
    const char* c_str() const;

private:
    char* data;
    u64 length_;
};