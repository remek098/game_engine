#pragma once

#include "defines.h"

#define LOG_WARN_ENABLED 1
#define LOG_INFO_ENABLED 1
#define LOG_DEBUG_ENABLED 1
#define LOG_TRACE_ENABLED 1

// Disable debg and trace logging for release builds.
#if KRELEASE == 1
#define LOG_DEBUG_ENABLED 0
#define LOG_TRACE_ENABLED 0
#endif



enum struct log_level {
    LOG_LEVEL_FATAL=   0,
    LOG_LEVEL_ERROR =  1,
    LOG_LEVEL_WARN =   2,
    LOG_LEVEL_INFO=    3,
    LOG_LEVEL_DEBUG=   4,
    LOG_LEVEL_TRACE=   5
};

/**
 * @brief Initializes logging system. Call twice; once with state = 0 to get required memory size,
 * then a second time passing allocated memory to state
 * 
 * @param memory_requirement a pointer to hold required memory size of internal state. 
 * @param state 0 if just requesting memory requirement, otherwise allocated block of memory.
 * @return b8 true on success; otherwise false;
 */
b8 initialize_logging(u64* memory_requirement, void* state);
void shutdown_logging(void* state);

KAPI void log_output(log_level level, const char* message, ...);

#define KFATAL(message, ...) log_output(log_level::LOG_LEVEL_FATAL, message, ##__VA_ARGS__);

#ifndef KERROR
#define KERROR(message, ...) log_output(log_level::LOG_LEVEL_ERROR, message, ##__VA_ARGS__);
#endif

#if LOG_WARN_ENABLED == 1
#define KWARN(message, ...) log_output(log_level::LOG_LEVEL_WARN, message,  ##__VA_ARGS__);
#else
#define KWARN(message, ...)
#endif


#if LOG_INFO_ENABLED == 1
#define KINFO(message, ...) log_output(log_level::LOG_LEVEL_INFO, message,  ##__VA_ARGS__);
#else
#define KINFO(message, ...)
#endif


#if LOG_DEBUG_ENABLED == 1
#define KDEBUG(message, ...) log_output(log_level::LOG_LEVEL_DEBUG, message,  ##__VA_ARGS__);
#else
#define KDEBUG(message, ...)
#endif


#if LOG_TRACE_ENABLED == 1
#define KTRACE(message, ...) log_output(log_level::LOG_LEVEL_TRACE, message,  ##__VA_ARGS__);
#else
#define KTRACE(message, ...)
#endif