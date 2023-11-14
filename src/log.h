#ifndef LOG_HEADER_H
#define LOG_HEADER_H

#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_YLW "\x1b[33m"
#define ANSI_COLOR_RST "\x1b[0m"
#define RED(str) ANSI_COLOR_RED str ANSI_COLOR_RST

#define LOGMETA    __FILE__, __LINE__, __PRETTY_FUNCTION__
#define NOLOGMETA    NULL,     -1,       NULL 

#define PRETTY_ERROR(subject, issue) fprintf(stderr, RED("[%s error]")" %s:%d in (%s): %s: %s\n", subject, LOGMETA, issue, strerror(errno)); exit(EXIT_FAILURE);
#define PRETTY_FAIL(subject, issue)  fprintf(stderr, RED("[%s error]")" %s:%d in (%s): %s\n", subject , LOGMETA, issue); exit(EXIT_FAILURE);

#define PRETTY_LOG_BUFFER_SIZE 2048

static void PRETTY_LOG(const char *subject, const char *filename, const ssize_t fileline, const char* function, const char *fmt, ...) {
    (void)subject;
    (void)filename;
    (void)fileline;
    (void)function;
    (void)fmt;
#ifdef VERBOSE
    va_list args;
    va_start(args, fmt);

    char strbuff[PRETTY_LOG_BUFFER_SIZE] = {};

    vsprintf(strbuff, fmt, args);

    if (filename && fileline != -1 && function) {
        fprintf(stderr, "[%s log] %s:%d in (%s): %s\n", subject, filename, (int)fileline, function, strbuff);
    } else {
        fprintf(stderr, "[%s log] %s\n", subject, strbuff);
    }

    va_end(args);
#endif
}

#endif  // LOG_HEADER_H

