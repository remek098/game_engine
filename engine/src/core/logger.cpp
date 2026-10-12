#include "logger.h"
#include "asserts.h"
#include "platform/platform.h"

// TODO: temporary
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

typedef struct logger_system_state {
    b8 initialized;
} logger_system_state;

static logger_system_state* state_ptr;

void report_assertion_failure(const char* expression, const char* message, const char* file, i32 line) {
    log_output(log_level::LOG_LEVEL_FATAL, "Assertion failure: %s, message: '%s', in file: %s, line: %d\n", expression, message, file, line);
}

b8 initialize_logging(u64* memory_requirement, void* state) {
    *memory_requirement = sizeof(logger_system_state);
    if(state == 0) {
        return true;
    }

    state_ptr = (logger_system_state*)state;
    state_ptr->initialized = true;

    // TODO: Remove this.
    KFATAL("Test message: %f", 3.14f);
    KERROR("Test message: %f", 3.14f);
    KWARN("Test message: %f", 3.14f);
    KINFO("Test message: %f", 3.14f);
    KDEBUG("Test message: %f", 3.14f);
    KTRACE("Test message: %f", 3.14f);

    // TODO: Create log file
    return true;
}

void shutdown_logging(void* state) {
    // TODO: Cleanup logging/write queued entries.
    state_ptr = 0;
}

void log_output(log_level level, const char* message, ...) {
    const char* level_strings[6] = {"[FATAL]: ", "[ERROR]: ", "[WARN]: ", "[INFO]: ", "[DEBUG]: ", "[TRACE]: "};
    b8 is_error = level < log_level::LOG_LEVEL_WARN; // < 2

    // Don't do that normally, doing that only so that we can avoid dynamic allocations for now
    const i32 msg_length = 32000;
    char out_message[msg_length];
    memset(out_message, 0, sizeof(out_message));

    // format original message
    __builtin_va_list arg_ptr;
    va_start(arg_ptr, message);
    vsnprintf(out_message, msg_length, message, arg_ptr);
    va_end(arg_ptr);

    char out_message2[msg_length];
    sprintf(out_message2, "%s%s\n", level_strings[(i32)level], out_message);
    
    // Platform-specific output.
    // printf("%s", out_message2);
    if(is_error) {
        platform::console_writeError(out_message2, (u8)level);
    }
    else {
        platform::console_write(out_message2, (u8)level);
    }
}