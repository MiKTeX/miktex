/*
 * Gregorio is a program that translates gabc files to GregorioTeX
 * This header prototypes the message logging functions.
 *
 * Copyright (C) 2009-2019 The Gregorio Project (see CONTRIBUTORS.md)
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

#ifndef MESSAGES_H
#define MESSAGES_H

#ifndef ENABLE_NLS
#define ENABLE_NLS 0
#endif
#if ENABLE_NLS == 1
#include "gettext.h"
#define _(str) gettext(str)
#define N_(str) (str)
#define ngt_(str, strtwo, count) ngettext(str, strtwo, count)
#else
#define _(str) (str)
#define N_(str) (str)
#define ngt_(str, strtwo, count) (str)
#endif

typedef enum gregorio_verbosity {
    VERBOSITY_INFO = 1,
    VERBOSITY_WARNING,
    VERBOSITY_DEPRECATION,
    VERBOSITY_ERROR,
    VERBOSITY_ASSERTION,
    VERBOSITY_FATAL
} gregorio_verbosity;

void gregorio_message(const char *string, const char *function_name,
        gregorio_verbosity verbosity, int line_number);
void gregorio_messagef(const char *function_name,
        gregorio_verbosity verbosity,
        int line_number, const char *format, ...)
        __attribute__ ((__format__ (__printf__, 4, 5)));
void gregorio_set_verbosity_mode(gregorio_verbosity verbosity);
void gregorio_set_debug_messages(bool debug);
void gregorio_set_deprecation_errors(bool deprecation_errors);
void gregorio_set_error_out(FILE *f);
int gregorio_get_return_value(void);

#define gregorio_assert_only(TEST,FUNCTION,MESSAGE) \
    if (!(TEST)) { \
        gregorio_message(_(MESSAGE), #FUNCTION, VERBOSITY_ASSERTION, __LINE__); \
    }

#define gregorio_assert(TEST,FUNCTION,MESSAGE,ON_FALSE) \
    if (!(TEST)) { \
        gregorio_message(_(MESSAGE), #FUNCTION, VERBOSITY_ASSERTION, __LINE__); \
        ON_FALSE; \
    }

#define gregorio_assert2(TEST,FUNCTION,FORMAT,ARG,ON_FALSE) \
    if (!(TEST)) { \
        gregorio_messagef(#FUNCTION, VERBOSITY_ASSERTION, __LINE__, FORMAT, ARG); \
        ON_FALSE; \
    }

#define gregorio_not_null(VARIABLE,FUNCTION,ON_FALSE) \
    gregorio_assert(VARIABLE, FUNCTION, #VARIABLE " may not be null", ON_FALSE)

#define gregorio_not_null_ptr(VARIABLE,FUNCTION,ON_FALSE) \
    gregorio_assert(VARIABLE && *VARIABLE, FUNCTION, #VARIABLE " may not be null", ON_FALSE)

#define gregorio_fail(FUNCTION,MESSAGE) \
    gregorio_message(_(MESSAGE), #FUNCTION, VERBOSITY_ASSERTION, __LINE__)

#define gregorio_fail2(FUNCTION,FORMAT,ARG) \
    gregorio_messagef(#FUNCTION, VERBOSITY_ASSERTION, __LINE__, FORMAT, ARG)

#endif
