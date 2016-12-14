#ifndef __LOG__H__
#define __LOG__H__

#include <stdio.h>
#include <stdarg.h>

/* 为了在关闭日志时能达到0开销，所以没法设计成能使用多个日志实例的形式 */

enum {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
    LOG_CRITICAL
};

#define LOG_LEVEL LOG_DEBUG

void log_set_file(FILE* fp);
FILE* log_get_file(void);

void log_log_valist(int level, const char* format, va_list ap);

static inline void log_debug(const char *format, ...);
static inline void log_info(const char *format, ...);
static inline void log_warning(const char *format, ...);
static inline void log_error(const char *format, ...);
static inline void log_critical(const char *format, ...);

static void log_debug(const char *format, ...)
{
#if LOG_LEVEL <= LOG_DEBUG
    va_list ap;
    va_start(ap, format);
    log_log_valist(LOG_DEBUG, format, ap);
    va_end(ap);
#endif
}

static void log_info(const char *format, ...)
{
#if LOG_LEVEL <= LOG_INFO
    va_list ap;
    va_start(ap, format);
    log_log_valist(LOG_INFO, format, ap);
    va_end(ap);
#endif
}

static void log_warning(const char *format, ...)
{
#if LOG_LEVEL <= LOG_WARNING
    va_list ap;
    va_start(ap, format);
    log_log_valist(LOG_WARNING, format, ap);
    va_end(ap);
#endif
}

static void log_error(const char *format, ...)
{
#if LOG_LEVEL <= LOG_ERROR
    va_list ap;
    va_start(ap, format);
    log_log_valist(LOG_ERROR, format, ap);
    va_end(ap);
#endif
}

static void log_critical(const char *format, ...)
{
#if LOG_LEVEL <= LOG_CRITICAL
    va_list ap;
    va_start(ap, format);
    log_log_valist(LOG_CRITICAL, format, ap);
    va_end(ap);
#endif
}

#endif /* __LOG__H__ */
