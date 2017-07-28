/* Pipestream: A simple C++ interface to UNIX pipes
   Version 0.05
   Copyright (C) 2005-2014 John C. Bowman,
   with contributions from Mojca Miklavec

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

#ifndef PIPESTREAM_H
#define PIPESTREAM_H

#if !defined(MIKTEX_WINDOWS)
#include <sys/wait.h>
#endif
#include <unistd.h>
#include <fcntl.h>

#include "common.h"
#if defined(MIKTEX_WINDOWS)
#include <miktex/PipeStream.h>
#include <cstddef>
typedef std::ptrdiff_t ssize_t;
#endif

// bidirectional stream for reading and writing to pipes
class iopipestream {
protected:
#if defined(MIKTEX_WINDOWS)
  MiKTeX::Aymptote::PipeStream pipeStream;
#else
  int in[2];
  int out[2];
#endif
  static const int BUFSIZE=SHRT_MAX;
  char buffer[BUFSIZE];
  string sbuffer;
  int pid;
  bool Running;
  bool pipeopen;
  bool pipein;
public:
  
  void open(const mem::vector<string> &command, const char *hint=NULL,
            const char *application="", int out_fileno=STDOUT_FILENO);

  bool isopen() {return pipeopen;}

  iopipestream(): pid(0), pipeopen(false) {}

  iopipestream(const mem::vector<string> &command, const char *hint=NULL,
               const char *application="", int out_fileno=STDOUT_FILENO) :
    pid(0), pipeopen(false) {
    open(command,hint,application,out_fileno);
  }

  void eof();
  virtual void pipeclose();

  virtual ~iopipestream() {
    pipeclose();
  }

  void block(bool write=false, bool read=true);
  
  ssize_t readbuffer();
  string readline();
  
  bool running() {return Running;}

  typedef iopipestream& (*imanip)(iopipestream&);

  iopipestream& operator << (imanip func) {
    return (*func)(*this);
  }

  iopipestream& operator >> (string& s) {
    readbuffer();
    s=buffer;
    return *this;
  }

  bool tailequals(const char *buf, size_t len, const char *prompt,
                  size_t plen);

  string getbuffer() {return sbuffer;}
  
  void wait(const char *prompt);
  int wait();
  void Write(const string &s);

  iopipestream& operator << (const string& s) {
    Write(s);
    return *this;
  }

  template<class T>
  iopipestream& operator << (T x) {
    ostringstream os;
    os << x;
    Write(os.str());
    return *this;
  }
};
  
#endif
