/* C++ interface to the XDR External Data Representation I/O routines
   Version 1.46
   Copyright (C) 1999-2007 John C. Bowman

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA. */

#ifndef __xstream_h__
#define __xstream_h__ 1

#if defined(HAVE_LIBTIRPC)

#include <cstdio>
#include <iostream>
#include <vector>
#include <cstdlib>
#if defined(_WIN32)
#include <fmem.h>
#endif

#include "xdrcommon.h"

#if defined(_WIN32)
typedef __int64 OffsetType;
#define fseeko _fseeki64
#define ftello _ftelli64
#else
#define OffsetType off_t
#endif

namespace xdr {

class xbyte {
  unsigned char c;
public:
  xbyte();
  xbyte(unsigned char c0);
  xbyte(int c0);
  xbyte(unsigned int c0);
  int byte() const;
  operator unsigned char () const;
};

class xios {
public:
  enum io_state {goodbit=0, eofbit=1, failbit=2, badbit=4};
  enum open_mode {in=1, out=2, app=8, trunc=16};
  enum seekdir {beg=SEEK_SET, cur=SEEK_CUR, end=SEEK_END};
private:
  int _state;
public:
  int good() const;
  int eof() const;
  int fail() const;
  int bad() const;
  void clear(int state = 0);
  void set(int flag);
  operator void*() const;
  int operator!() const;
};

class xstream : public xios {
protected:
  FILE *buf;
public:
  virtual ~xstream();
  xstream();

  void precision(int);

  xstream& seek(OffsetType pos, seekdir dir=beg);

  OffsetType tell();
};

#define IXSTREAM_DECL(T) ixstream& operator >> (T& x)
#define OXSTREAM_DECL(T) oxstream& operator << (T x)

class ixstream : virtual public xstream {
private:
  bool singleprecision;
protected:
  XDR xdri;
public:
  ixstream(bool singleprecision=false);

  virtual void open(const char *filename, open_mode=in);
  virtual void close();
  void closeFile();

  ixstream(const char *filename, bool singleprecision=false);
  ixstream(const char *filename, open_mode mode, bool singleprecision=false);
  virtual ~ixstream();

  typedef ixstream& (*imanip)(ixstream&);
  ixstream& operator >> (imanip func);

  IXSTREAM_DECL(int);
  IXSTREAM_DECL(unsigned int);
  IXSTREAM_DECL(long);
  IXSTREAM_DECL(unsigned long);
  IXSTREAM_DECL(long long);
  IXSTREAM_DECL(unsigned long long);
  IXSTREAM_DECL(short);
  IXSTREAM_DECL(unsigned short);
  IXSTREAM_DECL(char);
#ifndef _CRAY
  IXSTREAM_DECL(unsigned char);
#endif
  IXSTREAM_DECL(float);

  ixstream& operator >> (double& x);
  ixstream& operator >> (xbyte& x);
};

class oxstream : virtual public xstream {
private:
  bool singleprecision;
protected:
  XDR xdro;
public:
  oxstream(bool singleprecision=false);

  virtual void open(const char *filename, open_mode mode=trunc);

  virtual void close();

  void closefile();

  oxstream(const char *filename, bool singleprecision=false);

  oxstream(const char *filename, open_mode mode, bool singleprecision=false);
  virtual ~oxstream();

  oxstream& flush();

  typedef oxstream& (*omanip)(oxstream&);
  oxstream& operator << (omanip func);

  OXSTREAM_DECL(int);
  OXSTREAM_DECL(unsigned int);
  OXSTREAM_DECL(long);
  OXSTREAM_DECL(unsigned long);
  OXSTREAM_DECL(long long);
  OXSTREAM_DECL(unsigned long long);
  OXSTREAM_DECL(short);
  OXSTREAM_DECL(unsigned short);
  OXSTREAM_DECL(char);
#ifndef _CRAY
  OXSTREAM_DECL(unsigned char);
#endif
  OXSTREAM_DECL(float);

  oxstream& operator << (double x);

  oxstream& operator << (xbyte x);
};

class memoxstream : public oxstream
{
private:
#if defined(_WIN32)
  fmem fmInstance;
#else
  char* buffer=nullptr;
  size_t size=0;
#endif

public:
  memoxstream(bool singleprecision=false);
  ~memoxstream() override;
  std::vector<uint8_t> createCopyOfCurrentData();
};

class memixstream: public ixstream
{
public:
  memixstream(char* data, size_t length, bool singleprecision=false);

  explicit memixstream(std::vector<char>& data, bool singleprecision=false);

  ~memixstream() override;

  void close() override;

  void open(const char *filename, open_mode = in) override;
};

class ioxstream : public ixstream, public oxstream {
public:
  void open(const char *filename, open_mode mode=out) override;

  void close() override;

  ioxstream();
  ioxstream(const char *filename);
  ioxstream(const char *filename, open_mode mode);
  ~ioxstream() override;
};

oxstream& endl(oxstream& s);
oxstream& flush(oxstream& s);

#undef IXSTREAM_DECL
#undef OXSTREAM_DECL

}

#undef quad_t

#endif

#endif
