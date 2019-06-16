/*

Copyright 1996-2006 Han The Thanh <thanh@pdftex.org>
Copyright 2006-2009 Taco Hoekwater <taco@luatex.org>

This file is part of LuaTeX.

LuaTeX is free software; you can redistribute it and/or modify it under the terms
of the GNU General Public License as published by the Free Software Foundation;
either version 2 of the License, or (at your option) any later version.

LuaTeX is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU General Public License along with
LuaTeX; if not, see <http://www.gnu.org/licenses/>.

*/

#include "ptexlib.h"
#include "utils/avl.h"

/*tex

    Some memory management functions for AVL.

*/

static void *avl_xmalloc(struct libavl_allocator *allocator, size_t size)
{
    assert(allocator != NULL && size > 0);
    return xmalloc((unsigned) size);
}

static void avl_xfree(struct libavl_allocator *allocator, void *block)
{
    assert(allocator != NULL && block != NULL);
    xfree(block);
}

struct libavl_allocator avl_xallocator = {
    avl_xmalloc,
    avl_xfree
};

/*tex

    The general AVL comparison functions.

*/
int comp_int_entry(const void *pa, const void *pb, void *p)
{
    (void) p;
    cmp_return(*(const int *) pa, *(const int *) pb);
    return 0;
}

int comp_string_entry(const void *pa, const void *pb, void *p)
{
    (void) p;
    return strcmp((const char *) pa, (const char *) pb);
}
