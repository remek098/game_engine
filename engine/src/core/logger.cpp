#include "logger.h"
#include "asserts.h"
#include "platform/platform.h"

// TODO: temporary
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

void report_assertion_failure(const char* expression, const char* message, const char* file, i32 line) {
    log_output(log_level::LOG_LEVEL_FATAL, "Assertion failure: %s, message: '%s', in file: %s, line: %d\n", expression, message, file, line);
}

bool initialize_logging() {
    // TODO: Create log file
    return true;
}

void shutdown_logging() {
    // TODO: Cleanup logging/write queued entries.
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