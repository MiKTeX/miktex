/*
 * Gregorio is a program that translates gabc files to GregorioTeX
 * This header prototypes the miscellaneous support functions.
 *
 * Copyright (C) 2015-2019 The Gregorio Project (see CONTRIBUTORS.md)
 * 
 * This file is part of Gregorio.
 *
 * Gregorio is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Gregorio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Gregorio.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SUPPORT_H
#define SUPPORT_H

#include <stdlib.h>
#include <limits.h>
#ifdef USE_KPSE
#include <kpathsea/kpathsea.h>
#endif
#include "bool.h"
#include "config.h"
#include "messages.h"

#define GREGORIO_SZ_MAX (~((size_t)0))
#define MAX_BUF_GROWTH \
    ((size_t)((((INT_MAX < GREGORIO_SZ_MAX)? INT_MAX : GREGORIO_SZ_MAX) >> 1) + 1))

void gregorio_snprintf(char *s, size_t size, const char *format, ...)
        __attribute__((__format__ (__printf__, 3, 4)));
void *gregorio_malloc(size_t size) __attribute__((malloc));
void *gregorio_calloc(size_t nmemb, size_t size) __attribute__((malloc));
void *gregorio_realloc(void *ptr, size_t size)
    __attribute__((warn_unused_result));
char *gregorio_strdup(const char *s) __attribute__((malloc));
void *_gregorio_grow_buffer(void *buffer, size_t *nmemb, size_t size)
    __attribute__((warn_unused_result));
void gregorio_support_init(const char *program, const char *argv0);
void gregorio_print_version(const char *copyright);
char **gregorio_kpse_find(const char *filename);
void gregorio_exit(int status) __attribute__((noreturn));

#define gregorio_grow_buffer(BUFFER, NMEMB, TYPE) \
    ((TYPE *)_gregorio_grow_buffer(BUFFER, NMEMB, sizeof(TYPE)))

#ifdef USE_KPSE
bool gregorio_read_ok(const char *filename, gregorio_verbosity verbosity);
bool gregorio_write_ok(const char *filename, gregorio_verbosity verbosity);

#define gregorio_check_file_access(DIR, FILENAME, LEVEL, ON_NOT_ALLOWED) \
    if (!gregorio_ ## DIR ## _ok(FILENAME, VERBOSITY_ ## LEVEL)) { \
        ON_NOT_ALLOWED; \
    }
#define gregorio_kpse_find_or_else(VAR, FILENAME, ON_FAIL) \
    VAR = gregorio_kpse_find(FILENAME); \
    if (!VAR) { \
        ON_FAIL; \
    }
#else
#define gregorio_check_file_access(DIRECTION, FILENAME, LEVEL, ON_NOT_ALLOWED)
#define gregorio_kpse_find_or_else(VAR, FILENAME, ON_FAIL) \
    VAR = gregorio_kpse_find(FILENAME); \
    gregorio_not_null(VAR, gregorio_kpse_find, ON_FAIL)
#endif

#endif
