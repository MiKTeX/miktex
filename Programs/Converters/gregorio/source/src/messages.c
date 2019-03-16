/*
 * Gregorio is a program that translates gabc files to GregorioTeX
 * This file contains functions for logging messages, warnings, and errors.
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

#include "config.h"
#include <stdio.h>
#include <assert.h>
#include <stdarg.h>
#include "bool.h"
#include "messages.h"
#include "support.h"

static FILE *error_out;
static gregorio_verbosity verbosity_mode = 0;
static bool debug_messages = false;
static bool deprecation_is_warning = true;
static int return_value = 0;

int gregorio_get_return_value(void)
{
    return return_value;
}

void gregorio_set_error_out(FILE *const f)
{
    error_out = f;
}

void gregorio_set_verbosity_mode(const gregorio_verbosity verbosity)
{
    verbosity_mode = verbosity;
}

void gregorio_set_debug_messages(bool debug)
{
    debug_messages = debug;
}

void gregorio_set_deprecation_errors(bool deprecation_errors)
{
    deprecation_is_warning = !deprecation_errors;
}

static const char *verbosity_to_str(const gregorio_verbosity verbosity)
{
    const char *str;
    switch (verbosity) {
    case VERBOSITY_WARNING:
        str = _("warning:");
        break;
    case VERBOSITY_DEPRECATION:
        /* if there is no deprecation, these lines will not be hit */
        /* LCOV_EXCL_START */
        str = _("deprecation:");
        break;
        /* LCOV_EXCL_STOP */
    case VERBOSITY_ERROR:
        str = _("error:");
        break;
    case VERBOSITY_ASSERTION:
        /* not reachable unless there's a programming error */
        /* LCOV_EXCL_START */
        str = _("assertion:");
        break;
        /* LCOV_EXCL_STOP */
    case VERBOSITY_FATAL:
        /* all fatal errors should not be reasonably testable */
        /* LCOV_EXCL_START */
        str = _("fatal error:");
        break;
        /* LCOV_EXCL_STOP */
    default:
        /* INFO, for example */
        str = " ";
        break;
    }
    return str;
}

void gregorio_messagef(const char *function_name,
        gregorio_verbosity verbosity, int line_number,
        const char *format, ...)
{
    va_list args;
    const char *verbosity_str;

    if (!debug_messages && verbosity != VERBOSITY_ASSERTION) {
        line_number = 0;
        function_name = NULL;
    }

    /* if these assertions fail, the program is not using this code correctly */
    assert(error_out);
    assert(verbosity_mode);

    if (verbosity < verbosity_mode) {
        return;
    }
    if (verbosity == VERBOSITY_ASSERTION && return_value) {
        /* if something has already caused the system to fail, demote any
         * assertions coming after to warnings */
        verbosity = VERBOSITY_WARNING;
    }
    verbosity_str = verbosity_to_str(verbosity);
    if (line_number) {
        /* if line number is specified, function_name must be specified */
        assert(function_name);
        if (function_name) {
            fprintf(error_out, "%d: in function `%s': %s", line_number,
                    function_name, verbosity_str);
        }
    } else {
        if (function_name) {
            fprintf(error_out, "in function `%s': %s", function_name,
                    verbosity_str);
        } else {
            fprintf(error_out, "%s", verbosity_str);
        }
    }
    va_start(args, format);
    vfprintf(error_out, format, args);
    va_end(args);
    fprintf(error_out, "\n");

    switch (verbosity) {
    case VERBOSITY_DEPRECATION:
        /* if there is no deprecation, these lines will not be hit */
        /* LCOV_EXCL_START */
        if (deprecation_is_warning) {
            break;
        }
        /* LCOV_EXCL_STOP */
        /* else fall through */
    case VERBOSITY_ERROR:
    case VERBOSITY_ASSERTION:
        return_value = 1;
        break;
    case VERBOSITY_FATAL:
        /* all fatal errors should not be reasonably testable */
        /* LCOV_EXCL_START */
        gregorio_exit(1);
        break;
        /* LCOV_EXCL_STOP */
    default:
        break;
    }
}

void gregorio_message(const char *string, const char *function_name,
        gregorio_verbosity verbosity, int line_number)
{
    gregorio_messagef(function_name, verbosity, line_number, "%s", string);
}

