#pragma once

#include <defines.h>
#include <containers/darray.h>

#define BYPASS 2

// 1 -> true
// 0 -> false
// 2 -> BYPASS test
typedef u8 (*PFN_test)();

typedef struct test_entry {
    PFN_test func;
    const char* desc;
} test_entry;

class test_manager
{
public:
    test_manager();
    static void init();
    
    static void register_test(PFN_test, const char* desc);
    static void run_tests();

    static inline test_manager* get() {return manager;}
    static const darray<test_entry>& get_tests() {return manager->tests;}
private:
    static test_manager* manager;
    darray<test_entry> tests;
};