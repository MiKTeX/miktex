/**
 * @file subrange.cpp
 * @author Christian Schenk
 * @brief Subrange types
 *
 * @copyright Copyright © 1991-2022 Christian Schenk
 *
 * This file is part of C4P.
 *
 * C4P is licensed under GNU General Public License version 2 or any later
 * version.
 */

#include <cctype>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "common.h"

struct subrange_t
{
    C4P_integer lb;
    C4P_integer ub;
    const char * type_name;
};

namespace
{
    const subrange_t subrange_table[] =
    {
        -128l, 127l, "C4P_signed8",
        0l, 255l, "C4P_unsigned8",
        -32768, 32767l, "C4P_signed16",
        0l, 65535l, "C4P_unsigned16",
        -2147483648l, 2147483647l, "C4P_signed32"
    };
}

const char* subrange(C4P_integer lb, C4P_integer ub)
{
    const subrange_t* s = subrange_table;
    int i = sizeof(subrange_table) / sizeof(subrange_table[0]);
    for (; i--; ++s)
    {
        if (s->lb <= lb && lb <= s->ub && ub <= s->ub)
        {
            return s->type_name;
        }
    }
    return "C4P_unsigned32";
}
