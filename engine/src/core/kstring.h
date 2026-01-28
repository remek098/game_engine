#pragma once

#include "defines.h"

#include <stdio.h>
#include <stdarg.h>

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
    static bool are_equal(const char* str1, const char* str2);
    const char* c_str() const;

    static i32 format(char* dest, const char* format, ...);

    static i32 string_format_v(char* dest, const char* format, va_list args) ;

private:
    char* data;
    u64 length_;
};