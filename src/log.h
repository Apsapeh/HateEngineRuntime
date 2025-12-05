#pragma once

#include "types/types.h"

void log_init(void);
void log_exit(void);

/**
 * @brief Inner function to update the full trace info. Used for tracing with HEAPI_FULL_TRACE. Don't use
 directly
 *
 * @param func function_name, "" for reset
 * @param file "" for reset
 * @param line -1 for reset

 * @api
 */
void __he_update_full_trace_info(const char* func, const char* file, i32 line);

typedef struct {
    c_str func;
    c_str file;
    i32 line;
} log_full_trace_info;

log_full_trace_info log_full_trace_get_info(void);


#define LOG_FATAL_NO_ALLOC(...) __he_log_fatal_no_alloc(__LINE__, __FILE__, __VA_ARGS__);
#define LOG_ERROR_NO_ALLOC(...) __he_log_error_no_alloc(__LINE__, __FILE__, __VA_ARGS__);

// MACROS API BEGIN

#define LOG_INFO(...) __he_log_info(__LINE__, __FILE__, __VA_ARGS__)
#define LOG_WARN(...) __he_log_warning(__LINE__, __FILE__, __VA_ARGS__);
#define LOG_ERROR(...) __he_log_error(__LINE__, __FILE__, __VA_ARGS__);
#define LOG_FATAL(...) __he_log_fatal(__LINE__, __FILE__, __VA_ARGS__);

#ifdef HE_DEBUG
    #define LOG_DEBUG(...) __he_log_debug(__LINE__, __FILE__, __VA_ARGS__);
    #define LOG_WARN_OR_DEBUG_FATAL(...) LOG_WARN(__VA_ARGS__)
    #define LOG_ERROR_OR_DEBUG_FATAL(...) LOG_FATAL(__VA_ARGS__)
    #define LOG_ERROR_OR_DEBUG_FATAL_NO_ALLOC(...) LOG_FATAL_NO_ALLOC(__VA_ARGS__)
#else
    #define LOG_DEBUG(...)
    #define LOG_WARN_OR_DEBUG_FATAL(...) LOG_WARN(__VA_ARGS__)
// #define LOG_WARN_OR_FATAL_NO_ALLOC(...) LOG_WARN(__VA_ARGS__)
    #define LOG_ERROR_OR_DEBUG_FATAL(...) LOG_ERROR(__VA_ARGS__)
    #define LOG_ERROR_OR_DEBUG_FATAL_NO_ALLOC(...) LOG_ERROR_NO_ALLOC(__VA_ARGS__)
#endif

// MACROS API END

/**
 * @brief Inner function for logging in INFO mode. Don't use it directly
 *
 * @api
 */
void __he_log_info(i32 line, const char* file, const char* fmt, ...);

/**
 * @brief Inner function for logging in WARN mode. Don't use it directly
 *
 * @api
 */
void __he_log_warning(i32 line, const char* file, const char* fmt, ...);

/**
 * @brief Inner function for logging in FATAL mode. Don't use it directly
 *
 * @api
 */
void __he_log_fatal(i32 line, const char* file, const char* fmt, ...);

/**
 * @brief Inner function for logging in ERROR mode. Don't use it directly
 *
 * @api
 */
void __he_log_error(i32 line, const char* file, const char* fmt, ...);

/**
 * @brief Inner function for logging in DEBUG mode. Don't use it directly
 *
 * @api
 */
void __he_log_debug(i32 line, const char* file, const char* fmt, ...);


void __he_log_fatal_no_alloc(i32 line, const char* file, const char* fmt, ...);
void __he_log_error_no_alloc(i32 line, const char* file, const char* fmt, ...);
