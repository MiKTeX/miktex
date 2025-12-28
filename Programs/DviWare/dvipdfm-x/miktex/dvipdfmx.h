/**
 * @file miktex/dvipdfmx.h
 * @author Christian Schenk
 * @brief MiKTeX DVIPDFMx
 *
 * @copyright Copyright © 2016-2025 Christian Schenk
 *
 * This file is free software; the copyright holder gives unlimited permission
 * to copy and/or distribute it, with or without modifications, as long as this
 * notice is preserved.
 */

 #pragma once

#if defined(__cplusplus)
#include <cstdarg>
#else
#include <stdarg.h>
#endif

#if defined(__cplusplus)
extern "C" {
#endif

void miktex_log_error_va(const char* format, va_list args);
void miktex_log_info_va(const char* format, va_list args);
void miktex_log_warn_va(const char* format, va_list args);
void miktex_read_config_files();

#if defined(__cplusplus)
}
#endif
