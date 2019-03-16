/*
 * Copyright (C) 2006-2019 The Gregorio Project (see CONTRIBUTORS.md)
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

/* These macros are used for generating enum code.  This is based on the
 * technique described in http://stackoverflow.com/a/202511
 *
 * Two of these macros are meant for use outside this set of macros:
 *
 * - ENUM declares the enum itself and a prototype for the function that
 *   returns the string value of the enum's numeric value.  This is meant to be
 *   used from a header file.
 * - ENUM_TO_STRING defines the function whose prototype was declared by ENUM.
 *   This is meant to be used from a code file.
 *
 * These two macros both take the following two arguments:
 *
 * - TYPE is the name to use for the enum.
 * - DEF is the name of a macro that generates the bodies of the enum and the
 *   to-string function.  DEF should take four arguments, in the following
 *   order (with suggested names):
 *   - A is a macro that generates an enum value with a specifiec numeric
 *     value.  It takes two arguments, the name and the numeric value.
 *   - E is a macro that generates an enum value that has no specific numeric
 *     value.  It takes one argument, the name.
 *   - X is a macro that generates the last enum value when it has a specific
 *     numeric value.  It takes the same two arguments as A.
 *   - L is a macro that generates the last enum value when it has no specific
 *     numeric value.  It takes the same argument as E.
 *
 *  Note: If we were supporting C11, we could take empty macro arguments and
 *  would be able to get away with only two arguments to DEF.
 */

#ifndef ENUM_GENERATOR_H
#define ENUM_GENERATOR_H

/* for enum values */
#define ENUM_VALUE(NAME,VALUE) NAME = VALUE,
#define ENUM_ENTRY(NAME) NAME,
#define ENUM_LAST_VALUE(NAME,VALUE) NAME = VALUE
#define ENUM_LAST_ENTRY(NAME) NAME

/* for enum case */
#define ENUM_VALUE_CASE(NAME,VALUE) case NAME: return #NAME;
#define ENUM_ENTRY_CASE(NAME) case NAME: return #NAME;

/* enum declaration */
#define ENUM(TYPE,DEF) \
    typedef enum TYPE { \
        DEF(ENUM_VALUE, ENUM_ENTRY, ENUM_LAST_VALUE, ENUM_LAST_ENTRY) \
    } TYPE; \
    const char *TYPE##_to_string(TYPE value)

/* enum *_to_string function defintiion */
#define ENUM_TO_STRING(TYPE,DEF) \
    const char *TYPE##_to_string(TYPE value) \
    { \
        switch(value) { \
            DEF(ENUM_VALUE_CASE, ENUM_ENTRY_CASE, ENUM_VALUE_CASE, ENUM_ENTRY_CASE) \
            default: return gregorio_unknown(value); \
        } \
    }

#endif
