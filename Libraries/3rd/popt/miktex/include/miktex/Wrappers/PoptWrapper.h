/* miktex/Wrappers/PoptWrapper.h: popt library wrapper  -*- C++ -*-

   Copyright (C) 2001-2016 Christian Schenk

   This file is free software; you can redistribute it and/or modify
   it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   This file is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this file; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

#if defined(_MSC_VER)
#  pragma once
#endif

#if !defined(B67A80F5A5EE48459F83EE04616B8DFE)
#define B67A80F5A5EE48459F83EE04616B8DFE

#include <popt.h>

#define MIKTEX_POPT_BEGIN_NAMESPACE             \
  namespace MiKTeX {                            \
    namespace Wrappers {

#define MIKTEX_POPT_END_NAMESPACE               \
    }                                           \
  }

#include <string>
#include <vector>

MIKTEX_POPT_BEGIN_NAMESPACE;

class PoptWrapper
{
public:
  PoptWrapper()
  {
  }

public:
  PoptWrapper(const char * lpszName, int argc, const char ** argv, const struct poptOption * options, int flags = 0)
  {
    Construct(lpszName, argc, argv, options, flags);
  }

public:
  PoptWrapper(int argc, const char ** argv, const struct poptOption * options, int flags = 0)
  {
    Construct(argc, argv, options, flags);
  }

public:
  virtual ~PoptWrapper()
  {
    Dispose();
  }

public:
  void Construct(const char * lpszName, int argc, const char ** argv, const struct poptOption * options, int flags = 0)
  {
    ctx = poptGetContext(lpszName, argc, argv, options, flags);
  }

public:
  void Construct(int argc, const char ** argv, const struct poptOption * options, int flags = 0)
  {
    Construct(nullptr, argc, argv, options, flags);
  }

public:
  void Dispose()
  {
    if (ctx != nullptr)
    {
      poptFreeContext(ctx);
      ctx = nullptr;
    }
  }

public:
  operator poptContext () const
  {
    return ctx;
  }

public:
  operator poptContext ()
  {
    return ctx;
  }

public:
  void SetOtherOptionHelp(const char * lpsz)
  {
    poptSetOtherOptionHelp(ctx, lpsz);
  }

public:
  void PrintUsage(FILE * stream = stdout, int flags = 0) const
  {
    poptPrintUsage(ctx, stream, flags);
  }

public:
  void PrintHelp(FILE * stream = stdout, int flags = 0) const
  {
    poptPrintHelp(ctx, stream, flags);
  }

public:
  int GetNextOpt()
  {
    return poptGetNextOpt(ctx);
  }

public:
  std::string GetOptArg()
  {
    std::string result;
    char * lpsz = poptGetOptArg(ctx);
    if (lpsz != nullptr)
    {
      result = lpsz;
      free(lpsz);
    }
    return result;
  }

public:
  std::vector<std::string> GetLeftovers()
  {
    std::vector<std::string> result;
    const char ** args = poptGetArgs(ctx);
    if (args != nullptr)
    {
      for (const char ** a = args; *a != nullptr; ++a)
      {
        result.push_back(*a);
      }
    }
    return result;
  }

public:
  std::string BadOption(int flags)
  {
    return poptBadOption(ctx, flags);
  }

public:
  static std::string Strerror(int error)
  {
    return poptStrerror(error);
  }

private:
  poptContext ctx = nullptr;
};

MIKTEX_POPT_END_NAMESPACE;

#endif
