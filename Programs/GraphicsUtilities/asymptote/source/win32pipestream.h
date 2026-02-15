/**
 * @file win32pipestream.h
 * @brief A reimplementation of pipestream for win32
 * @author Supakorn "Jamie" Rassameemasmuang (jamievlin [at] outlook.com)
 */

#pragma once

#if defined(_WIN32)
#include "common.h"
#include <array>
#include <iomanip>

#include <Windows.h>

namespace w32
{

class Win32IoPipeStream
{
public:
  static const size_t BUFFER_LEN = SHRT_MAX;

  void open(
          const mem::vector<string> &command,
          const char *hint=NULL,
          const char *application="",
          int out_fileno=STDOUT_FILENO);

  virtual ~Win32IoPipeStream();

  [[nodiscard]]
  bool isopen() const;

  Win32IoPipeStream() = default;
  Win32IoPipeStream(
          const mem::vector<string> &command,
          const char *hint=NULL,
          const char *application="",
          int out_fileno=STDOUT_FILENO);

  void eof();
  virtual void pipeclose();
  void block(bool write, bool read);
  int wait();

  void Write(const string &s);
  void wait(const char *prompt);
  string readline();

  bool tailequals(
          const char *buf,
          size_t len,
          const char *prompt,
          size_t plen);

  ssize_t readbuffer();

  Win32IoPipeStream& operator>>(string& s);
  Win32IoPipeStream& operator<<(const string& s);

  typedef Win32IoPipeStream& (*imanip)(Win32IoPipeStream&);
  Win32IoPipeStream& operator<< (imanip func);

  string getbuffer();
  bool running();

  template<class T>
  Win32IoPipeStream& operator<<(T x) {
    ostringstream os;
    os << x;
    Write(os.str());
    return *this;
  }

private:
  void closeProcessHandles();

protected:
  PROCESS_INFORMATION procInfo = {};
  bool pipeopen = false;

  HANDLE processStdinWr;
  HANDLE processOutRd;
  bool Running;
  bool pipein;

  char buffer[BUFFER_LEN];
  string sbuffer;
};

}

#endif
