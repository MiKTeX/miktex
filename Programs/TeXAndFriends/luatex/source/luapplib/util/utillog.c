
#include <stdio.h>
#include <string.h> // strlen
#include <stdarg.h>
#include "utillog.h"

#define LOGGER_BUFFER_SIZE 256
#define LOGGER_PREFIX_SIZE 32

typedef struct {
  logger_function callback;
  void *context;
  size_t pfxlen;
} logger_struct;

static logger_struct logger = { 0, NULL, 0 };

static char logger_buffer[LOGGER_BUFFER_SIZE+LOGGER_PREFIX_SIZE];

void loggerf (const char *format, ...)
{
  va_list args;
  int length;
  
  va_start(args, format);  
  length = vsnprintf(logger_buffer + logger.pfxlen, LOGGER_BUFFER_SIZE, format, args);
	if (length > 0)
	{
		if (length > LOGGER_BUFFER_SIZE)
			length = LOGGER_BUFFER_SIZE;
	}
	else
	{
		loggerf("logger encoding error '%s'", format);
		length = (int)strlen(logger_buffer);
	}
	length += (int)logger.pfxlen;
	if (logger.callback)
		logger.callback(logger_buffer, logger.context);
  else
		printf("\n%s\n", logger_buffer);
	va_end(args);
}

void logger_callback (logger_function callback, void *context)
{
	logger.callback = callback;
	logger.context = context;
}

int logger_prefix (const char *prefix)
{
	size_t pfxlen;
	pfxlen = strlen(prefix);
	if (pfxlen > LOGGER_PREFIX_SIZE)
		return 0;
	memcpy(logger_buffer, prefix, pfxlen);
	logger.pfxlen = pfxlen;
	return 1;
}
