// -*- related-file-name: "../include/lcdf/slurper.hh" -*-

/* slurper.{cc,hh} -- reading from files a line at a time
 *
 * Copyright (c) 1998-2019 Eddie Kohler
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version. This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <lcdf/slurper.hh>
#include <string.h>

static const int DefChunkCap	= 2048;
static const int WorthMoving	= 256;


Slurper::Slurper(const Filename &filename, FILE *f)
  : _filename(filename), _lineno(0),
    _data(new unsigned char[DefChunkCap]), _cap(DefChunkCap),
    _pos(0), _len(0), _line(0), _line_len(0),
    _saved_line(false), _at_eof(false)
{
  if (f) {
    _f = f;
    _own_f = false;
  } else {
    _f = _filename.open_read();
    _own_f = true;
  }
}

Slurper::~Slurper()
{
  delete[] _data;
  if (_f && _own_f) fclose(_f);
}


void
Slurper::grow_buffer()
{
  // If we need to keep the upper part of the buffer, shift it down (moving
  // any data we care about).
  if (_pos >= _cap - WorthMoving) {
    // I used to move it myself, with a comment that SunOS didn't have a
    // correct implementation of memmove. This should be detected by
    // configure, if it's really a problem.
    memmove(_data, _data + _pos, _len - _pos);
    _len -= _pos;
    _pos = 0;
  }

  // Grow the buffer if necessary.
  if (_len >= _cap) {
    unsigned char *new_data = new unsigned char[ 2 * _cap ];
    // I can't believe I didn't have the line below!!
    memcpy(new_data, _data, _len);
    delete[] _data;
    _data = new_data;
    _cap = 2 * _cap;
  }
}


inline int
Slurper::more_data()
{
  grow_buffer();

  // Read new data into the buffer.
  int amount = fread(_data + _len, 1, _cap - _len, _f);
  _len += amount;
  return amount;
}

char *
Slurper::get_line_at(unsigned pos)
{
  while (1) {
    for (; pos < _len; pos++)
      if (_data[pos] == '\n' || _data[pos] == '\r')
	goto line_ends_at_pos;

    // no line end? look for more data. save and reset `pos', since _pos
    // may change.
    int offset = pos - _pos;
    bool got_more_data = more_data() != 0;
    pos = _pos + offset;
    if (!got_more_data) {
      _at_eof = true;
      goto line_ends_at_pos;
    }
  }

 line_ends_at_pos:

  // PRECONDITION: the line starts at _pos and ends at pos.
  unsigned next_pos;

  // Find beginning of next line. 3 cases:
  // 1. line ends in \r\n	-> _pos = pos + 2;
  // 2. line ends in \r OR \n	-> _pos = pos + 1;
  // 3. neither			-> must be last line in file; _pos = pos,
  //				   since pos == _len
  if (pos == _len) {
    // last line in file didn't end in `\n': must have no data left
    // ensure we have enough space for terminating nul
    if (pos == _cap) grow_buffer();
    next_pos = pos;
    // if already at EOF, don't increment the line number
    if (pos == _pos) _lineno--;

  } else if (_data[pos] == '\n')
    next_pos = pos + 1;

  else {
    assert(_data[pos] == '\r');
    // If `\r' is last char in buffer, `\n' might be next char. Must read more
    // data to check for it, or we'd report an empty line that didn't really
    // exist.
    if (pos == _len - 1) {
      // be careful about the possible shift of _pos and _len!
      int offset = pos - _pos;
      more_data();
      pos = _pos + offset;
    }
    if (pos < _len - 1 && _data[pos + 1] == '\n')
      next_pos = pos + 2;
    else
      next_pos = pos + 1;
  }

  _line = _data + _pos;
  _line_len = pos - _pos;
  _data[pos] = 0;
  _pos = next_pos;
  _lineno++;
  return (char *)_line;
}

char *
Slurper::next_line()
{
  if (_saved_line) {
    _saved_line = false;
    return (char *)_line;
  }
  get_line_at(_pos);
  if (_line_len == 0 && _at_eof)
    _line = 0;
  return (char *)_line;
}

char *
Slurper::peek_line()
{
  next_line();
  _saved_line = true;
  return (char *)_line;
}

char *
Slurper::append_next_line()
{
  unsigned delta = (_data + _pos) - (_line + _line_len);
  if (_len - _pos > _line_len) {
    memmove(_line + delta, _line, _line_len);
    _line += delta;
  } else {
    memmove(_data + _pos - delta, _data + _pos, _len - _pos);
    _pos -= delta;
    _len -= delta;
  }

  unsigned append_at = _pos;
  _pos = _line - _data;
  return get_line_at(append_at);
}
