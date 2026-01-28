#include "test_manager.h"

// #include <containers/darray.h>
#include <core/logger.h>
#include <core/kstring.h>
#include <core/clock.h>

test_manager* test_manager::manager = nullptr;

test_manager::test_manager() {
    manager = this;
}

void test_manager::init() {
    test_manager::get()->tests.create();
}

void test_manager::register_test(u8 (*PFN_test)(), const char* desc) {
    test_entry e;

    e.func = PFN_test;
    e.desc = desc;
    test_manager::get()->tests.push(e);
}

void test_manager::run_tests() {
    u32 passed = 0;
    u32 failed = 0;
    u32 skipped = 0;

    u32 count = get_tests().get_length();

    kclock total_time;
    total_time.start();

    for(u32 i = 0; i < count; ++i) {
        kclock test_time;
        test_time.start();
        // KINFO("Running '%s'", get_tests()[i].desc);
        u8 result = get_tests()[i].func();
        test_time.update();

        if(result == true) {
            ++passed;
        }
        else if(result == BYPASS) {
            KWARN("[SKIPPED]: %s", get_tests()[i].desc);
            ++skipped;
        }
        else {
            KERROR("[FAILED]: %s", get_tests()[i].desc);
            ++failed;
        }

        char status[20];
        kstring::format(status, failed ? "*** %d FAILED ***" : "SUCCESS", failed);
        total_time.update();
        KINFO("Executed %d of %d (skipped %d) %s (%.6f sec / %.6f sec total)", i + 1, count, skipped, status, test_time.elapsed, total_time.elapsed);
    }

    total_time.stop();
    KINFO("Results: %d passed, %d failed, %d skipped.", passed, failed, skipped);
}