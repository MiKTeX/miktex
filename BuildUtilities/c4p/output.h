/* output.h: c4p output routines                        -*- C++ -*-

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

class output
{
private:
  void flush_buf();

public:
  unsigned get_buf_mark()
  {
    return buf_ptr;
  }

public:
  void get_buf_text(std::string & str, unsigned buf_mark, unsigned count);

public:
  void out_buf_over(unsigned buf_mark, const char * s, unsigned count);

public:
  void open_file(unsigned filnum, const char * file_name);

public:
  void close_file(unsigned filnum);

public:
  void reopen_file(unsigned filnum, const char * file_name);

private:
  void put_char(int c);

private:
  void out_char(int c);

public:
  void out_stop()
  {
    halting = true;
  }

public:
  void out_resume()
  {
    halting = false;
  }

public:
  void out_s(const std::string & s);

public:
  void redir_file(int filnum);

public:
  void translate_char(int c);

public:
  void translate_string(const char * s);

private:
  bool halting;

private:
  FILE * file_table[3];

private:
  FILE * cur_file;

private:
  int cur_file_num;

private:
  char out_buf[BUF_SIZE];

private:
  unsigned buf_ptr;

private:
  unsigned chars_in_buf;

private:
  int last_char;
};

extern output cppout;
