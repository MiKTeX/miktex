/* dvitype-miktex.h:                                    -*- C++ -*-

   Copyright (C) 1991-2016 Christian Schenk

   This file is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published
   by the Free Software Foundation; either version 2, or (at your
   option) any later version.

   This file is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this file; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA. */

#if defined(_MSC_VER)
#  pragma once
#endif

#include <miktex/TeXAndFriends/config.h>

#if !defined(THEDATA)
#  define THEDATA(x) C4P_VAR(x)
#endif

#if defined(MIKTEX_WINDOWS)
#  include "dvitype.rc"
#endif

#include <miktex/TeXAndFriends/WebApp>

#define OPT_OUT_MODE 1000
#define OPT_START_THERE 1001
#define OPT_MAX_PAGES 1002
#define OPT_RESOLUTION 1003
#define OPT_NEW_MAG 1004

using namespace MiKTeX::TeXAndFriends;
using namespace MiKTeX::Core;

class DVITYPECLASS :
  public WebApp
{
public:
  void AddOptions() override
  {
    AddOption(MIKTEXTEXT("out-mode\0Set output mode."), OPT_OUT_MODE, POPT_ARG_STRING, "MODE");
    AddOption(MIKTEXTEXT("start-there\0Set starting page."), OPT_START_THERE, POPT_ARG_STRING, "PAGESPEC");
    AddOption(MIKTEXTEXT("max-pages\0Set maximum number of pages."), OPT_MAX_PAGES, POPT_ARG_STRING, "N");
    AddOption(MIKTEXTEXT("resolution\0Set desired resolution."), OPT_RESOLUTION, POPT_ARG_STRING, MIKTEXTEXT("NUM/DEN"));
    AddOption(MIKTEXTEXT("new-mag\0Set new magnification."), OPT_NEW_MAG, POPT_ARG_STRING, "MAG");
    WebApp::AddOptions();
  }

public:
  const char * GetUsage() const override
  {
    return MIKTEXTEXT("[OPTION...] INPUTFILE");
  }

public:
  void Init(const std::string & programInvocationName) override
  {
    WebApp::Init(programInvocationName);
    THEDATA(outmode) = 4;
    THEDATA(startvals) = 0;
    THEDATA(startthere)[0] = false;
    THEDATA(maxpages) = 1000000;
    THEDATA(resolution) = 300.0;
    THEDATA(newmag) = 0;
  }

public:
  bool ProcessOption(int opt, const std::string & optArg) override
  {
    bool done = true;
    switch (opt)
    {
    case OPT_OUT_MODE:
      if (!(optArg == "0" || optArg == "1" || optArg == "2" || optArg == "3" || optArg == "4"))
      {
        BadUsage();
      }
      THEDATA(outmode) = std::stoi(optArg);
      break;
    case OPT_START_THERE:
    {
      const char * lpsz = optArg.c_str();
      size_t k = 0;
      do
      {
        if (*lpsz == MIKTEXTEXT('*'))
        {
          THEDATA(startthere)[k] = false;
          ++lpsz;
        }
        else if (!(isdigit(*lpsz) || (*lpsz == MIKTEXTEXT('-') && isdigit(lpsz[1]))))
        {
          BadUsage();
        }
        else
        {
          THEDATA(startthere)[k] = true;
          char * lpsz2 = 0;
          THEDATA(startcount)[k] = strtol(lpsz, &lpsz2, 10);
          lpsz = const_cast<const char *>(lpsz2);
        }
        if (k < 9 && *lpsz == '.')
        {
          ++k;
          ++lpsz;
        }
        else if (*lpsz == 0)
        {
          THEDATA(startvals) = static_cast<C4P_signed8>(k);
        }
        else
        {
          BadUsage();
        }
      } while (THEDATA(startvals) != static_cast<C4P_signed8>(k));
    }
    break;
    case OPT_MAX_PAGES:
      if (optArg.empty() || !isdigit(optArg[0]))
      {
        BadUsage();
      }
      THEDATA(maxpages) = std::stoi(optArg);
      break;
    case OPT_RESOLUTION:
    {
      int num, den;
#if defined(MIKTEX_WINDOWS)
      if (sscanf_s(optArg.c_str(), "%d/%d", &num, &den) != 2 || (num < 0))
      {
        BadUsage();
      }
#else
      if (sscanf(optArg.c_str(), "%d/%d", &num, &den) != 2 || (num < 0))
      {
        BadUsage();
      }
#endif
      THEDATA(resolution) = static_cast<float>(num) / den;
    }
    break;
    case OPT_NEW_MAG:
      if (optArg.empty() || !isdigit(optArg[0]))
      {
        BadUsage();
      }
      THEDATA(newmag) = std::stoi(optArg);
      break;
    default:
      done = WebApp::ProcessOption(opt, optArg);
      break;
    }
    return done;
  }

public:
  void ProcessCommandLineOptions() override
  {
    WebApp::ProcessCommandLineOptions();
    if (C4P::GetArgC() != 2)
    {
      BadUsage();
    }
  }
};

extern DVITYPECLASS DVITYPEAPP;
#define THEAPP DVITYPEAPP

#include <miktex/TeXAndFriends/WebApp.inl>
