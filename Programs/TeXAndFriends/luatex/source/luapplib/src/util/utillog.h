
#ifndef UTIL_LOG_H
#define UTIL_LOG_H

typedef void (*logger_function) (const char *message, void *alien);
void loggerf (const char *format, ...);
void logger_callback (logger_function callback, void *context);
int logger_prefix (const char *prefix);

#endif