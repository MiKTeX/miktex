/* bibtex-miktex.h:                                     -*- C++ -*-

   Copyright (C) 1996-2017 Christian Schenk

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

#include "bibtex-miktex-config.h"

#define IMPLEMENT_TCX 1

#include <miktex/Core/FileType>
#include <miktex/Core/Registry>
#include <miktex/TeXAndFriends/CharacterConverterImpl>
#include <miktex/TeXAndFriends/InitFinalizeImpl>
#include <miktex/TeXAndFriends/InputOutputImpl>
#include <miktex/TeXAndFriends/WebAppInputLine>

#if !defined(MIKTEXHELP_BIBTEX)
#  include <miktex/Core/Help>
#endif

#include "bibtex.h"

extern BIBTEXPROGCLASS BIBTEXPROG;

class BIBTEXAPPCLASS :
  public MiKTeX::TeXAndFriends::WebAppInputLine
{
public:
  template<typename T> T* Reallocate(T*& p, size_t n)
  {
    size_t amount = n * sizeof(T);
    p = reinterpret_cast<T*>(realloc(p, amount));
    if (p == nullptr && amount > 0)
    {
      FatalError(MIKTEXTEXT("Virtual memory exhausted."));
    }
    return p;
  }
  
private:
  template<typename T> T* Allocate(T*&  p, size_t n)
  {
    p = nullptr;
    return Reallocate(p, n);
  }

private:
  template<typename T> T* Free(T*& p)
  {
    return Reallocate(p, 0);
  }

private:
  std::shared_ptr<MiKTeX::Core::Session> session;
  
private:
  MiKTeX::TeXAndFriends::CharacterConverterImpl<BIBTEXPROGCLASS> charConv{ BIBTEXPROG };

private:
  MiKTeX::TeXAndFriends::InitFinalizeImpl<BIBTEXPROGCLASS> initFinalize{ BIBTEXPROG };

private:
  MiKTeX::TeXAndFriends::InputOutputImpl<BIBTEXPROGCLASS> inputOutput{ BIBTEXPROG };

public:
  void Init(std::vector<char*>& args) override
  {
    SetCharacterConverter(&charConv);
    SetInitFinalize(&initFinalize);
    SetInputOutput(&inputOutput);
    WebAppInputLine::Init(args);
    session = GetSession();
#if defined(IMPLEMENT_TCX)
    EnableFeature(MiKTeX::TeXAndFriends::Feature::TCX);
#endif
    BIBTEXPROG.mincrossrefs = session->GetConfigValue(MIKTEX_REGKEY_BIBTEX, "min_crossrefs", 2).GetInt();
    BIBTEXPROG.maxbibfiles = 20;
    BIBTEXPROG.maxentints = 3000;
    BIBTEXPROG.maxentstrs = 3000;
    BIBTEXPROG.maxfields = 100000; //5000;
    BIBTEXPROG.poolsize = 65000;
    BIBTEXPROG.wizfnspace = 3000;
    Allocate(BIBTEXPROG.bibfile, BIBTEXPROG.maxbibfiles);
    Allocate(BIBTEXPROG.biblist, BIBTEXPROG.maxbibfiles);
    Allocate(BIBTEXPROG.entryints, BIBTEXPROG.maxentints);
    Allocate(BIBTEXPROG.entrystrs, BIBTEXPROG.maxentstrs);
    Allocate(BIBTEXPROG.fieldinfo, BIBTEXPROG.maxfields);
    Allocate(BIBTEXPROG.spreamble, BIBTEXPROG.maxbibfiles);
    Allocate(BIBTEXPROG.strpool, BIBTEXPROG.poolsize);
    Allocate(BIBTEXPROG.wizfunctions, BIBTEXPROG.wizfnspace);
  }
  
public:
  void Finalize() override
  {
    Free(BIBTEXPROG.bibfile);
    Free(BIBTEXPROG.biblist);
    Free(BIBTEXPROG.entryints);
    Free(BIBTEXPROG.entrystrs);
    Free(BIBTEXPROG.fieldinfo);
    Free(BIBTEXPROG.spreamble);
    Free(BIBTEXPROG.strpool);
    Free(BIBTEXPROG.wizfunctions);
    WebAppInputLine::Finalize();
  }

#define OPT_MIN_CROSSREFS 1000
#define OPT_QUIET 1001

public:
  void AddOptions() override
  {
    WebAppInputLine::AddOptions();
    AddOption(MIKTEXTEXT("min-crossrefs\0Include item after N cross-refs; default 2."), OPT_MIN_CROSSREFS, POPT_ARG_STRING, "N");
    AddOption(MIKTEXTEXT("quiet\0Suppress all output (except errors)."), OPT_QUIET, POPT_ARG_NONE);
    AddOption("silent", "quiet");
    AddOption("terse", "quiet");
  }
  
public:
  MiKTeX::Core::FileType GetInputFileType() const override
  {
    return MiKTeX::Core::FileType::BIB;
  }

public:
  std::string MIKTEXTHISCALL GetUsage() const override
  {
    return MIKTEXTEXT("[OPTION...] AUXFILE");
  }

public:
  bool ProcessOption(int opt, const std::string& optArg) override
  {
    bool done = true;
    switch (opt)
      {
      case OPT_MIN_CROSSREFS:
        BIBTEXPROG.mincrossrefs = std::stoi(optArg);
        break;
      case OPT_QUIET:
        SetQuietFlag(true);
        break;
      default:
        done = WebAppInputLine::ProcessOption(opt, optArg);
        break;
      }
    return done;
  }
  
public:
  std::string MIKTEXTHISCALL TheNameOfTheGame() const override
  {
    return "BibTeX";
  }

public:
  unsigned long MIKTEXTHISCALL GetHelpId() const override
  {
    return MIKTEXHELP_BIBTEX;
  }

public:
  void BufferSizeExceeded() const override
  {
    std::cout << "Sorry---you've exceeded BibTeX's buffer size";
    GetInitFinalize()->history() = 3;
    c4pthrow(9998);
  }

public:
  template<class T> bool OpenBstFile(T& f) const
  {
    const char* lpszFileName = GetInputOutput()->nameoffile();
    MIKTEX_ASSERT_STRING(lpszFileName);
    MiKTeX::Core::PathName bstFileName(lpszFileName);
    if (!bstFileName.HasExtension())
    {
      bstFileName.SetExtension(".bst");
    }
    MiKTeX::Core::PathName path;
    if (!session->FindFile(bstFileName.ToString(), MiKTeX::Core::FileType::BST, path))
    {
      return false;
    }
    FILE* file = session->OpenFile(path.GetData(), MiKTeX::Core::FileMode::Open, MiKTeX::Core::FileAccess::Read, true);
    f.Attach(file, true);
#ifdef PASCAL_TEXT_IO
    get(f);
#endif
    return true;
  }
};

extern BIBTEXAPPCLASS BIBTEXAPP;

template<class T> inline void miktexbibtexrealloc(const char* varName, T*& p, size_t n)
{
  p = BIBTEXAPP.Reallocate(p, n + 1);
}

template<class T> inline bool miktexopenbstfile(T& f)
{
  return BIBTEXAPP.OpenBstFile(f);
}

inline bool miktexhasextension(const char* fileName, const char* extension)
{
  return MiKTeX::Core::PathName(fileName).HasExtension(extension);
}
