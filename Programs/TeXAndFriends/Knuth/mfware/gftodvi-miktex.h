/* gftodvi-miktex.h:                                    -*- C++ -*-

   Copyright (C) 1991-2017 Christian Schenk

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

#include "gftodvi-miktex-config.h"

#include <miktex/TeXAndFriends/WebApp>

#include "gftodvi.h"

#define OPT_OVERFLOW_LABEL_OFFSET 1000
#define OPT_TITLEFONT 1001
#define OPT_LABELFONT 1002
#define OPT_GRAYFONT 1003
#define OPT_SLANTFONT 1004
#define OPT_LOGOFONT 1005

#if !defined(MIKTEXHELP_GFTODVI)
#  include <miktex/Core/Help>
#endif

extern GFTODVIPROGCLASS GFTODVIPROG;

class GFTODVIAPPCLASS :
  public MiKTeX::TeXAndFriends::WebApp
{
public:
  void MIKTEXTHISCALL AddOptions() override
  {
    AddOption(MIKTEXTEXT("overflow-label-offset\0Specifies the distance from the right edge of the character bounding box at which the overflow equations (if any) are typeset. The value is given in TeX points."), OPT_OVERFLOW_LABEL_OFFSET, POPT_ARG_STRING, "REAL");
    AddOption(MIKTEXTEXT("title-font\0Font for titles."), OPT_TITLEFONT, POPT_ARG_STRING, "FONT");
    AddOption(MIKTEXTEXT("label-font\0Font for labels."), OPT_LABELFONT, POPT_ARG_STRING, "FONT");
    AddOption(MIKTEXTEXT("gray-font\0Font for pixels."), OPT_GRAYFONT, POPT_ARG_STRING, "FONT");
    AddOption(MIKTEXTEXT("slant-font\0Font for slants."), OPT_SLANTFONT, POPT_ARG_STRING, "FONT");
    AddOption(MIKTEXTEXT("logo-font\0Font for METAFONT logo."), OPT_LOGOFONT, POPT_ARG_STRING, "FONT");
    WebApp::AddOptions();
  }

public:
  std::string GetUsage() const override
  {
    return MIKTEXTEXT("[OPTION...] [INPUTFILE]");
  }

public:
  void Init(std::vector<char*>& args) override
  {
    WebApp::Init(args);
    GFTODVIPROG.overlabeloffset = 10000000;
    titleFont = "cmr8";
    labelFont = "cmtt10";
    grayFont = "gray";
    slantFont = "";
    logoFont = "logo8";
  }

public:
  bool ProcessOption(int opt, const std::string& optArg) override
  {
    bool done = true;
    switch (opt)
    {
    case OPT_OVERFLOW_LABEL_OFFSET:
      GFTODVIPROG.overlabeloffset = C4P::Round(std::stof(optArg) * 65536.0);
      break;
    case OPT_TITLEFONT:
      titleFont = optArg;
      break;
    case OPT_LABELFONT:
      labelFont = optArg;
      break;
    case OPT_GRAYFONT:
      grayFont = optArg;
      break;
    case OPT_SLANTFONT:
      slantFont = optArg;
      break;
    case OPT_LOGOFONT:
      logoFont = optArg;
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
    int argc = C4P::GetArgC();
    const char** argv = C4P::GetArgV();
    if (argc > 1)
    {
      inputFile = argv[1];
    }
  }

public:
  bool GetInputFileName()
  {
    if (inputFile.empty() || inputFile.length() > GFTODVIPROG.terminallinelength)
    {
      return false;
    }
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
    strcpy_s(reinterpret_cast<char*>(GFTODVIPROG.buffer), GFTODVIPROG.terminallinelength, inputFile.c_str());
#else
    strcpy(reinterpret_cast<char*>(GFTODVIPROG.buffer), inputFile.c_str());
#endif
    GFTODVIPROG.linelength = static_cast<C4P_unsigned8>(inputFile.length());
    inputFile = "";
    return true;
  }

private:
  GFTODVIPROGCLASS::strnumber MakeString(const char* lpsz)
  {
    size_t l = strlen(lpsz);
    if (l == 0)
    {
      return 0;
    }
    memcpy(GFTODVIPROG.strpool + GFTODVIPROG.poolptr, lpsz, l);
    GFTODVIPROG.poolptr += static_cast<GFTODVIPROGCLASS::poolpointer>(l);
    GFTODVIPROG.strptr += 1;
    GFTODVIPROG.strstart[GFTODVIPROG.strptr] = GFTODVIPROG.poolptr;
    return GFTODVIPROG.strptr - 1;
  }

public:
  GFTODVIPROGCLASS::strnumber GetTitleFont()
  {
    return MakeString(titleFont.c_str());
  }

public:
  GFTODVIPROGCLASS::strnumber GetLabelFont()
  {
    return MakeString(labelFont.c_str());
  }

public:
  GFTODVIPROGCLASS::strnumber GetGrayFont()
  {
    return MakeString(grayFont.c_str());
  }

public:
  GFTODVIPROGCLASS::strnumber GetSlantFont()
  {
    return MakeString(slantFont.c_str());
  }

public:
  GFTODVIPROGCLASS::strnumber GetLogoFont()
  {
    return MakeString(logoFont.c_str());
  }

private:
  std::string inputFile;

private:
  std::string titleFont;

private:
  std::string labelFont;

private:
  std::string grayFont;

private:
  std::string slantFont;

private:
  std::string logoFont;

public:
  unsigned long GetHelpId() const override
  {
    return MIKTEXHELP_GFTODVI;
  }
};

extern GFTODVIAPPCLASS GFTODVIAPP;

inline bool miktexgetinputfilename()
{
  return GFTODVIAPP.GetInputFileName();
}

inline GFTODVIPROGCLASS::strnumber miktexgettitlefont()
{
  return GFTODVIAPP.GetTitleFont();
}

inline GFTODVIPROGCLASS::strnumber miktexgetlabelfont()
{
  return GFTODVIAPP.GetLabelFont();
}

inline GFTODVIPROGCLASS::strnumber miktexgetgrayfont()
{
  return GFTODVIAPP.GetGrayFont();
}

inline GFTODVIPROGCLASS::strnumber miktexgetslantfont()
{
  return GFTODVIAPP.GetSlantFont();
}

inline GFTODVIPROGCLASS::strnumber miktexgetlogofont()
{
  return GFTODVIAPP.GetLogoFont();
}
