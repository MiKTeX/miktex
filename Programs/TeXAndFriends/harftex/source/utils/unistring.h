/* unistring.h

   Copyright 2013 Taco Hoekwater <taco@luatex.org>

   This file is part of LuaTeX.

   LuaTeX is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 2 of the License, or (at your
   option) any later version.

   LuaTeX is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
   License for more details.

   You should have received a copy of the GNU General Public License along
   with LuaTeX; if not, see <http://www.gnu.org/licenses/>. */


#ifndef UNISTRING_H
#  define UNISTRING_H

extern unsigned char *uni2str(unsigned);
extern unsigned str2uni(const unsigned char *);
extern int buffer_to_unichar(int k);

extern char *uni2string(char *utf8_text, unsigned ch);
extern unsigned u_length(register unsigned int *str);
extern void utf2uni_strcpy(unsigned int *ubuf, const char *utf8buf); 

#define is_utf8_follow(A) (A >= 0x80 && A < 0xC0)

#define utf8_size(a) (a>0xFFFF ? 4 : (a>0x7FF ? 3 : (a>0x7F? 2 : 1)))

extern char *utf16be_str(long code);

#endif

