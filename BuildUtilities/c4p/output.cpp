/* output.cpp: c4p output routines                      -*- C++ -*-

   Copyright (C) 1991-2016 Christian Schenk

   This file is part of C4P.

   C4P is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   C4P is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with C4P; if not, write to the Free Software Foundation, 59
   Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#include <algorithm>

#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <cstdlib>

#include "common.h"

#include "output.h"

output cppout;

void output::flush_buf()
{
  if (chars_in_buf == BUF_SIZE)
  {
    fwrite(out_buf + buf_ptr, 1, BUF_SIZE - buf_ptr, cur_file);
  }
  if (buf_ptr > 0)
  {
    fwrite(out_buf, 1, buf_ptr, cur_file);
  }
  buf_ptr = 0;
  chars_in_buf = 0;
}

void output::get_buf_text(std::string & str, unsigned buf_mark, unsigned count)
{
  buf_mark %= BUF_SIZE;
  if (count == 0)
  {
    if (buf_mark < buf_ptr)
    {
      count = buf_ptr - buf_mark;
    }
    else
    {
      count = BUF_SIZE - (buf_mark - buf_ptr);
    }
  }
  unsigned n = std::min(count, (unsigned)BUF_SIZE - buf_mark);
  str.assign(out_buf + buf_mark, n);
  if (n < count)
  {
    str.append(out_buf, count - n);
  }
}

void output::out_buf_over(unsigned buf_mark, const char * s, unsigned count)
{
  if (count == 0)
  {
    if (buf_mark < buf_ptr)
    {
      count = buf_ptr - buf_mark;
    }
    else
    {
      count = BUF_SIZE - (buf_mark - buf_ptr);
    }
  }
  unsigned n = std::min(count, (unsigned)BUF_SIZE - buf_mark);
  memcpy(out_buf + buf_mark, s, n);
  if (n < count)
  {
    memcpy(out_buf, s + n, count - n);
  }
}

void output::open_file(unsigned filnum, const char * file_name)
{
  if ((file_table[filnum] = fopen(file_name, "wb")) == nullptr)
  {
    perror(file_name);
  }
  if (filnum == C_FILE_NUM)
  {
    c_file_line_count = 0;
  }
}

void output::close_file(unsigned filnum)
{
  flush_buf();
  fclose(file_table[filnum]);
}

void output::reopen_file(unsigned filnum, const char * file_name)
{
  flush_buf();
  fclose(file_table[filnum]);
  open_file(filnum, file_name);
}

void output::put_char(int c)
{
  if (chars_in_buf == BUF_SIZE)
  {
    fputc(out_buf[buf_ptr], cur_file);
  }
  else
  {
    ++chars_in_buf;
  }
  out_buf[buf_ptr] = (char)c;
  buf_ptr = (buf_ptr + 1) % BUF_SIZE;
}

void output::out_char(int c)
{
  if (last_char == '\n')
  {
    for (unsigned i = 0; i < (curly_brace_level + extra_indent) * 2; ++i)
    {
      put_char(' ');
    }
    if (cur_file_num == C_FILE_NUM)
    {
      ++c_file_line_count;
    }
  }
  if (c == '\n' && macroizing)
  {
    put_char('\\');
  }
  put_char(c);
  last_char = c;
}

void output::out_s(const std::string & s)
{
  if (halting)
  {
    return;
  }
  for (const char & ch : s)
  {
    out_char(ch);
    last_char = ch;
  }
}

void output::redir_file(int filnum)
{
  flush_buf();
  if (filnum >= 0)
  {
    cur_file = file_table[filnum];
    last_char = '\n';
  }
  cur_file_num = filnum;
}

void output::translate_char(int c)
{
  switch (c)
  {
  case '\'':
    out_s("\\\'");
    break;
  case '\\':
    out_s("\\\\");
    break;
  default:
    out_char(c);
    break;
  }
}

void output::translate_string(const char * s)
{
  while (*s)
  {
    switch (*s)
    {
    case '"':
      out_s("\\\"");
      break;
    case '\\':
      out_s("\\\\");
      break;
    default:
      out_char(*s);
      break;
    }
    ++s;
  }
}
