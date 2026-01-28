#include "test_manager.h"

#include "memory/linear_allocator_tests.h"

#include <core/logger.h>

int main() {
    test_manager test_man;
    test_man.init();

    // TODO: register tests
    linear_allocator_register_tests();
    
    
    KDEBUG("Stating tests...");
    test_man.run_tests();

    return 0;
}