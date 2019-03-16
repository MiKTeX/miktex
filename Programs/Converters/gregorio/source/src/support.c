/*
 * Gregorio is a program that translates gabc files to GregorioTeX
 * This file contains miscellaneous support functions.
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

#include "config.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "support.h"
#include "struct.h"
#include "messages.h"

/* Our version of snprintf; this is NOT semantically the same as C99's
 * snprintf; rather, it's a "lowest common denominator" implementation
 * blending C99 and MS-C */
void gregorio_snprintf(char *s, size_t size, const char *format, ...)
{
    va_list args;

#ifdef _MSC_VER
    memset(s, 0, size);
#endif

    va_start(args, format);
#ifdef _MSC_VER
    _vsnprintf_s(s, size, _TRUNCATE, format, args);
#else
    vsnprintf(s, size, format, args);
#endif
    va_end(args);
}

static __inline void *assert_successful_allocation(void *ptr, char *funcname) {
    if (!ptr) {
        /* it's not realistic to test this for coverage  */
        /* LCOV_EXCL_START */
        gregorio_message(_("error in memory allocation"),
                funcname, VERBOSITY_FATAL, 0);
        gregorio_exit(1);
    }
    /* LCOV_EXCL_STOP */
    return ptr;
}

void *gregorio_malloc(size_t size)
{
    return assert_successful_allocation(malloc(size), "gregorio_malloc");
}

void *gregorio_calloc(size_t nmemb, size_t size)
{
    return assert_successful_allocation(calloc(nmemb, size), "gregorio_calloc");
}

void *gregorio_realloc(void *ptr, size_t size)
{
    return assert_successful_allocation(realloc(ptr, size), "gregorio_realloc");
}

char *gregorio_strdup(const char *s)
{
    return (char *)assert_successful_allocation(strdup(s), "gregorio_strdup");
}

void *_gregorio_grow_buffer(void *buffer, size_t *nmemb, size_t size)
{
    if (buffer == NULL) {
        return gregorio_malloc(*nmemb * size);
    }
    if (*nmemb >= MAX_BUF_GROWTH) {
        /* it's not realistic to test this case */
        /* LCOV_EXCL_START */
        gregorio_message(_("buffer too large"), "gregorio_grow_buffer",
                VERBOSITY_FATAL, 0);
        gregorio_exit(1);
        /* LCOV_EXCL_STOP */
    }
    *nmemb <<= 1;
    return gregorio_realloc(buffer, *nmemb * size);
}

#ifdef USE_KPSE
static kpathsea kpse = NULL;
#define USED_FOR_KPSE
#else
#define USED_FOR_KPSE __attribute__((unused))
#endif

void gregorio_support_init(const char *const program USED_FOR_KPSE,
        const char *const argv0 USED_FOR_KPSE)
{
    gregorio_set_error_out(stderr);
    gregorio_set_verbosity_mode(VERBOSITY_ERROR);
    gregorio_struct_init();
#ifdef USE_KPSE
    kpse = kpathsea_new();
    kpathsea_set_program_name(kpse, argv0, program);
#endif
}

void gregorio_print_version(const char *copyright)
{
#ifdef USE_KPSE
    printf("Gregorio %s (%s).\n%s\n", GREGORIO_VERSION,
            kpathsea_version_string, copyright);
#else
    printf("Gregorio %s.\n%s\n", GREGORIO_VERSION,
            copyright);
#endif
    printf("License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\n\
This is free software: you are free to change and redistribute it.\n\
There is NO WARRANTY, to the extent permitted by law.\n");
}

#ifdef USE_KPSE
bool gregorio_read_ok(const char *const filename,
        const gregorio_verbosity verbosity)
{
    if (kpathsea_in_name_ok_silent(kpse, filename)) {
        return true;
    }
    gregorio_messagef("gregorio_in_name_ok", verbosity, __LINE__,
            _("kpse prohibits read from file %s"), filename);
    return false;
}

bool gregorio_write_ok(const char *const filename,
        const gregorio_verbosity verbosity)
{
    if (kpathsea_out_name_ok_silent(kpse, filename)) {
        return true;
    }
    gregorio_messagef("gregorio_out_name_ok", verbosity, __LINE__,
            _("kpse prohibits write to file %s"), filename);
    return false;
}
#endif

#ifndef USE_KPSE
static __inline void rtrim(char *buf)
{
    char *p;
    for (p = buf + strlen(buf) - 1; p >= buf && isspace(*p); --p) {
        *p = '\0';
    }
}

static bool gregorio_readline(char **buf, size_t *bufsize, FILE *file)
{
    size_t oldsize;
    if (*buf == NULL) {
        *bufsize = 128;
        *buf = gregorio_grow_buffer(NULL, bufsize, char);
    } else {
        if (*bufsize < 128) {
            /* not reachable unless there's a programming error */
            /* LCOV_EXCL_START */
            gregorio_message(_("invalid buffer size"), "gregorio_getline",
                    VERBOSITY_FATAL, 0);
            gregorio_exit(1);
            /* LCOV_EXCL_STOP */
        }
    }
    (*buf)[0] = '\0';
    oldsize = 1;
    for (;;) {
        (*buf)[*bufsize - 2] = '\0';

        if (feof(file) || ferror(file)
                || !fgets((*buf) + oldsize - 1, (int)(*bufsize - oldsize + 1),
                    file)
                || (*buf)[*bufsize - 2] == '\0') {
            if (ferror(file)) {
                /* it's not realistic to simulate the system error required to
                 * cover this case */
                /* LCOV_EXCL_START */
                gregorio_message(_("Error reading from the file"),
                        "gregorio_getline", VERBOSITY_FATAL, 0);
                gregorio_exit(1);
                /* LCOV_EXCL_STOP */
            }
            return (*buf)[0] != '\0';
        }

        oldsize = *bufsize;
        *buf = gregorio_grow_buffer(*buf, bufsize, char);
    }
}
#endif


char **gregorio_kpse_find(const char *filename)
{
    char **filenames;
#ifdef USE_KPSE
    filenames = kpathsea_find_file_generic(kpse, filename, kpse_tex_format,
            true, true);
    if (!filenames) {
        /* It's not reasonable to break kpse in such a way that this would
         * fail. */
        /* LCOV_EXCL_START */
        gregorio_messagef("gregorio_kpse_find", VERBOSITY_WARNING, 0,
                _("kpathsea_find_file_generic returned NULL: %s"), filename);
        return NULL;
        /* LCOV_EXCL_STOP */
    }
#else
    FILE *file;
    size_t bufsize = 0;
    char *buf = NULL;
    size_t capacity = 16, size = 0;
#define KPSE_COMMAND "kpsewhich -must-exist -all "
    size_t command_size = sizeof(KPSE_COMMAND) + strlen(filename);
    char *command = gregorio_malloc(command_size);

    strcpy(command, KPSE_COMMAND);
    strcpy(command + sizeof(KPSE_COMMAND) - 1, filename);
    file = popen(command, "r");
    free(command);
    command = NULL; /* just to be sure */
    if (!file) {
        /* it's not reasonable to cause popen to fail */
        /* LCOV_EXCL_START */
        gregorio_messagef("gregorio_kpse_find", VERBOSITY_WARNING, 0,
                _("unable to run kpsewhich %s: %s"), filename,
                strerror(errno));
        return NULL;
        /* LCOV_EXCL_STOP */
    }
    filenames = gregorio_malloc(capacity * sizeof(char *));
    while (gregorio_readline(&buf, &bufsize, file)) {
        rtrim(buf);
        if (strlen(buf) > 0) {
            filenames[size++] = gregorio_strdup(buf);
            if (size >= capacity) {
                capacity <<= 1;
                filenames = gregorio_realloc(filenames,
                                capacity * sizeof(char *));
            }
        } else {
            gregorio_messagef("read_patterns", VERBOSITY_WARNING, 0,
                    _("kpsewhich returned bad value for %s"), filename);
        }
    }
    free(buf);
    filenames[size] = NULL;
    pclose(file);
#endif
    return filenames;
}

void gregorio_exit(int status)
{
#ifdef USE_KPSE
    if (kpse) {
        kpathsea_finish(kpse);
    }
#endif
    gregorio_struct_destroy();
    exit(status);
} /* the prior line exits; LCOV_EXCL_LINE */
