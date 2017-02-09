/* omega-miktex.h:                                      -*- C++ -*-

   Copyright (C) 1998-2017 Christian Schenk

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

#include "omega-miktex-config.h"

#include <miktex/KPSE/Emulation>
#include <miktex/TeXAndFriends/CharacterConverterImpl>
#include <miktex/TeXAndFriends/InitFinalizeImpl>
#include <miktex/TeXAndFriends/InputOutputImpl>
#include <miktex/TeXAndFriends/TeXApp>
#include <miktex/TeXAndFriends/TeXMemoryHandlerImpl>
#include <miktex/W2C/Emulation>

#if defined(MIKTEX)
#define P1H(x1) (x1)
#define P1C(t1, v1) (t1 v1)
#define P2C(t1, v1, t2, v2) (t1 v1, t2 v2)
#define P4C(t1, v1, t2, v2, t3, v3, t4, v4) (t1 v1, t2 v2, t3 v3, t4 v4)
#endif

#include "omega.h"

#if defined(MIKTEX_WINDOWS)
#  include "omega.rc"
#endif

namespace omega {
#include <miktex/omega.defaults.h>
}

#if !defined(MIKTEXHELP_OMEGA)
#  include <miktex/Core/Help>
#endif

extern OMEGAPROGCLASS OMEGAPROG;

class MemoryHandlerImpl :
  public MiKTeX::TeXAndFriends::TeXMemoryHandlerImpl<OMEGAPROGCLASS>
{
public:
  MemoryHandlerImpl(OMEGAPROGCLASS& program, MiKTeX::TeXAndFriends::TeXMFApp& texmfapp) :
    TeXMemoryHandlerImpl<OMEGAPROGCLASS>(program, texmfapp)
  {
  }

public:
  void Allocate(const std::unordered_map<std::string, int>& userParams) override
  {
    TeXMemoryHandlerImpl<OMEGAPROGCLASS>::Allocate(userParams);
    program.ocpbufsize = GetParameter("ocp_buf_size", userParams, omega::omega::ocp_buf_size());
    program.ocplistinfosize = GetParameter("ocp_listinfo_size", userParams, omega::omega::ocp_listinfo_size());
    program.ocplistlistsize = GetParameter("ocp_list_list_size", userParams, omega::omega::ocp_list_list_size());
    program.ocplstacksize = GetParameter("ocp_lstack_size", userParams, omega::omega::ocp_lstack_size());
    program.ocpstacksize = GetParameter("ocp_stack_size", userParams, omega::omega::ocp_stack_size());
    AllocateArray("inputfilemode", program.inputfilemode, program.maxinopen);
    AllocateArray("inputfiletranslation", program.inputfiletranslation, program.maxinopen);
    AllocateArray("ocplistinfo", program.ocplistinfo, program.ocplistinfosize);
    AllocateArray("ocplistlist", program.ocplistlist, program.ocplistlistsize);
    AllocateArray("ocplstackinfo", program.ocplstackinfo, program.ocplstacksize);
    AllocateArray("strstartar", program.strstartar, program.maxstrings);
    AllocateArray("triec", program.triec, program.triesize);
    AllocateArray("trieoplang", program.trieoplang, program.trieopsize);
  }

public:
  void Free() override
  {
    TeXMemoryHandlerImpl<OMEGAPROGCLASS>::Free();
    FreeArray("inputfilemode", program.inputfilemode);
    FreeArray("inputfiletranslation", program.inputfiletranslation);
    FreeArray("ocplistinfo", program.ocplistinfo);
    FreeArray("ocplistlist", program.ocplistlist);
    FreeArray("ocplstackinfo", program.ocplstackinfo);
    FreeArray("strstartar", program.strstartar);
    FreeArray("triec", program.triec);
    FreeArray("trieoplang", program.trieoplang);
  }

public:
  void Check() override
  {
    TeXMemoryHandlerImpl<OMEGAPROGCLASS>::Check();
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.inputfilemode);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.inputfiletranslation);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.ocplistinfo);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.ocplistlist);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.ocplstackinfo);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.strstartar);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.triec);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.trieoplang);
  }
};

class OMEGAAPPCLASS :
  public MiKTeX::TeXAndFriends::TeXApp
{
protected:
  void AddOptions() override
  {
    TeXApp::AddOptions();
    AddOption("oft", "undump");
  }

private:
  MiKTeX::TeXAndFriends::CharacterConverterImpl<OmegaProgram> charConv{ OMEGAPROG };

private:
  MiKTeX::TeXAndFriends::InitFinalizeImpl<OmegaProgram> initFinalize{ OMEGAPROG };

private:
  MiKTeX::TeXAndFriends::InputOutputImpl<OmegaProgram> inputOutput{ OMEGAPROG };

private:
  MemoryHandlerImpl memoryHandler{ OMEGAPROG, *this };

private:
  std::shared_ptr<MiKTeX::Core::Session> session;

public:
  void Init(const std::string& programInvocationName) override
  {
    SetCharacterConverter(&charConv);
    SetInitFinalize(&initFinalize);
    SetInputOutput(&inputOutput);
    SetTeXMFMemoryHandler(&memoryHandler);
    TeXApp::Init(programInvocationName);
    session = GetSession();
#if defined(IMPLEMENT_TCX)
    EnableFeature(Feature::TCX);
#endif
  }

public:
  MiKTeX::Core::PathName GetMemoryDumpFileName() const override
  {
    return "omega.fmt";
  }

public:
  const char * GetPoolFileName() const
  {
    return "omega.pool";
  }

public:
  std::string GetInitProgramName() const override
  {
    return "iniomega";
  }

public:
  std::string GetVirginProgramName() const override
  {
    return "viromega";
  }

public:
  std::string TheNameOfTheGame() const override
  {
    return "Omega";
  }

public:
  unsigned long GetHelpId() const override
  {
    return MIKTEXHELP_OMEGA;
  }

public:
  bool OpenOCPFile(OMEGAPROGCLASS::bytefile& f, char* lpszFileName)
  {
    MiKTeX::Core::PathName fileName;
    if (!session->FindFile(lpszFileName, MiKTeX::Core::FileType::OCP, fileName))
    {
      return false;
    }
    FILE* file = session->TryOpenFile(fileName.GetData(), MiKTeX::Core::FileMode::Open, MiKTeX::Core::FileAccess::Read, false);
    if (file == nullptr)
    {
      return false;
    }
    f.Attach(file, true);
    get(f);
    return true;
  }
  
public:
  bool OpenONMFile(OMEGAPROGCLASS::alphafile& f, char* lpszFileName)
  {
#if 1
    return false;
#else
    MiKTeX::Core::PathName fileName;
    if (!session->FindFile(lpszFileName, MiKTeX::Core::FileType::ONM, fileName))
    {
      return false;
    }
    FILE* file = session->OpenFile(fileName.Get(), MiKTeX::Core::FileMode::Open, MiKTeX::Core::FileAccess::Read, false);
    if (file == nullptr)
    {
      return false;
    }
    f.Attach(file, true);
#if defined(PASCAL_TEXT_IO)
    get(f);
#endif
    return true;
#endif
  }
};

int miktexloadpoolstrings(int size);

inline int loadpoolstrings(int size)
{
  return miktexloadpoolstrings(size);
}

extern OMEGAAPPCLASS OMEGAAPP;

#define uexit(exitCode) throw exitCode

inline bool miktexopenocpfile(OMEGAPROGCLASS::bytefile& f, char* n)
{
  return OMEGAAPP.OpenOCPFile(f, n);
}

inline bool miktexopenonmfile(OMEGAPROGCLASS::alphafile& f, char* n)
{
  return OMEGAAPP.OpenONMFile(f, n);
}

inline void dumpwd(OMEGAPROGCLASS::memoryword wd)
{
  *OMEGAPROG.fmtfile = wd;
  put(OMEGAPROG.fmtfile);
}

inline void dumpint(C4P::C4P_integer i)
{
  (*OMEGAPROG.fmtfile).c4p_P2.c4p_int = i;
  put(OMEGAPROG.fmtfile);
}

inline void undumpwd(OMEGAPROGCLASS::memoryword& wd)
{
  get(OMEGAPROG.fmtfile);
  wd = *OMEGAPROG.fmtfile;
}

inline void undumpint(C4P::C4P_integer& i)
{
  get(OMEGAPROG.fmtfile);
  i = (*OMEGAPROG.fmtfile).c4p_P2.c4p_int;
}

template<typename T> int dumpthings(const T& first_item, std::size_t n)
{
  return fwrite(&first_item, sizeof(first_item), n, OMEGAPROG.fmtfile);
}

template<typename T> int undumpthings(T& first_item, std::size_t n)
{
  return fread(&first_item, sizeof(first_item), n, OMEGAPROG.fmtfile);
}

#define cint c4p_P2.c4p_int
#define cint1 c4p_P2.hh.c4p_P1.lh

using alphafile = OMEGAPROGCLASS::alphafile;
using halfword = OMEGAPROGCLASS::halfword;
using memoryword = OMEGAPROGCLASS::memoryword;

extern C4P::C4P_signed32& first;
extern OMEGAPROGCLASS::memoryword**& fonttables;
extern OMEGAPROGCLASS::memoryword**& fontsorttables;
extern C4P::C4P_signed32& last;
extern C4P::C4P_signed16& namelength;
extern C4P::C4P_integer& ocpbufsize;
extern C4P::C4P_integer**& ocptables;
extern OMEGAPROGCLASS::quarterword* otpinputbuf;
extern OMEGAPROGCLASS::halfword& otpinputend;
extern OMEGAPROGCLASS::quarterword* otpoutputbuf;
extern OMEGAPROGCLASS::halfword& otpoutputend;

extern char* nameoffile;

inline auto ziniteqtbentry(OMEGAPROGCLASS::halfword p)
{
  return OMEGAPROG.initeqtbentry(p);
}

inline auto pnewinputln(OMEGAPROGCLASS::alphafile f, OMEGAPROGCLASS::halfword themode, OMEGAPROGCLASS::halfword translation, C4P::C4P_boolean bypasseoln)
{
  return OMEGAPROG.pnewinputln(f, themode, translation, bypasseoln);
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   the rest is omegamem.h in the web2c world; non-obvious MiKTeX
   changes are marked
   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

extern boolean new_input_line (alphafile &, halfword); // MiKTeX
extern boolean newinputln (alphafile &, halfword, halfword, boolean); // MiKTeX
extern int getc_two ();
extern int getfilemode (alphafile &, int def); // MiKTeX
extern void ungetc_two ();
extern int ocptemp;

#define newinputline(stream, mode, flag)   new_input_line (stream, mode)

#ifndef MIKTEX
extern memoryword **fonttables;
extern memoryword **fontsorttables;
extern int **ocptables;
#endif

typedef struct hashw
{
  integer p;
  struct hashw *ptr;
  memoryword mw;
} hashword;

extern hashword hashtable[];
extern hashword *createhashpos(int); // MiKTeX
extern hashword *createeqtbpos(int); // MiKTeX
extern hashword *createxeqlevel(int); // MiKTeX
extern void inithhashtable();
extern void dumphhashtable();
extern void undumphhashtable();
extern void allocatefonttable(int, int); // MiKTeX
extern void dumpfonttable(int, int); // MiKTeX
extern void undumpfonttable(int); // MiKTeX
extern void allocatefontsorttable(int, int); // MiKTeX
extern void dumpfontsorttable(int, int); // MiKTeX
extern void undumpfontsorttable(int); // MiKTeX
extern void allocateocptable(int, int); // MiKTeX
extern void dumpocptable(int); // MiKTeX
extern void undumpocptable(int); // MiKTeX
#if 0                           // MiKTeX
extern void odateandtime();
#endif
extern void btestin();
extern void runexternalocp(char *); // MiKTeX

#define initeqtbtable()         inithhashtable();
#define dumpeqtbtable()         dumphhashtable();
#define undumpeqtbtable()       undumphhashtable();

#define HASHTABLESIZE           23123
#define neweqtb(a)              (((a)==hashtable[(a)%HASHTABLESIZE].p) ? \
                                 (hashtable[(a)%HASHTABLESIZE].mw) : \
                                 (createeqtbpos(a)->mw))
#define neweqtbint(a)           (((a)==hashtable[(a)%HASHTABLESIZE].p) ? \
                                 (hashtable[(a)%HASHTABLESIZE].mw.cint) : \
                                 (createeqtbpos(a)->mw.cint))
#define neweqtbsc(a)            (((a)==hashtable[(a)%HASHTABLESIZE].p) ? \
                                 (hashtable[(a)%HASHTABLESIZE].mw.cint) : \
                                 (createeqtbpos(a)->mw.cint))
#define newequiv(a)             (((a)==hashtable[(a)%HASHTABLESIZE].p) ? \
                                 (hashtable[(a)%HASHTABLESIZE].mw.c4p_P2.hh.rh) : \
                                 (createeqtbpos(a)->mw.c4p_P2.hh.rh))
#define newequiv1(a)            (((a)==hashtable[(a)%HASHTABLESIZE].p) ? \
                        (hashtable[(a)%HASHTABLESIZE].mw.cint1) : \
                                 (createeqtbpos(a)->mw.cint1))
#define neweqlevel(a)           (((a)==hashtable[(a)%HASHTABLESIZE].p) ? \
                                 (hashtable[(a)%HASHTABLESIZE].mw.c4p_P2.hh.c4p_P1.c4p_P0.b1) : \
                                 (createeqtbpos(a)->mw.c4p_P2.hh.c4p_P1.c4p_P0.b1))
#define neweqtype(a)            (((a)==hashtable[(a)%HASHTABLESIZE].p) ? \
                                 (hashtable[(a)%HASHTABLESIZE].mw.c4p_P2.hh.c4p_P1.c4p_P0.b0) : \
                                 (createeqtbpos(a)->mw.c4p_P2.hh.c4p_P1.c4p_P0.b0))
#define setneweqtb(a,v)         (((a)==hashtable[(a)%HASHTABLESIZE].p) ? \
                                 (hashtable[(a)%HASHTABLESIZE].mw=v) : \
                                 (createeqtbpos(a)->mw=v))
#define setneweqtbint(a,v)      (((a)==hashtable[(a)%HASHTABLESIZE].p) ? \
                                 (hashtable[(a)%HASHTABLESIZE].mw.cint=v) : \
                                 (createeqtbpos(a)->mw.cint=v))
#define setneweqtbsc(a,v)       (((a)==hashtable[(a)%HASHTABLESIZE].p) ? \
                                 (hashtable[(a)%HASHTABLESIZE].mw.cint=v) : \
                                 (createeqtbpos(a)->mw.cint=v))
#define setequiv(a,v)           (((a)==hashtable[(a)%HASHTABLESIZE].p) ? \
                                 (hashtable[(a)%HASHTABLESIZE].mw.c4p_P2.hh.rh=v) : \
                                 (createeqtbpos(a)->mw.c4p_P2.hh.rh=v))
#define setequiv1(a,v)          (((a)==hashtable[(a)%HASHTABLESIZE].p) ? \
                         (hashtable[(a)%HASHTABLESIZE].mw.cint1=v) : \
                                 (createeqtbpos(a)->mw.cint1=v))
#define seteqlevel(a,v)         (((a)==hashtable[(a)%HASHTABLESIZE].p) ? \
                                 (hashtable[(a)%HASHTABLESIZE].mw.c4p_P2.hh.c4p_P1.c4p_P0.b1=v) : \
                                 (createeqtbpos(a)->mw.c4p_P2.hh.c4p_P1.c4p_P0.b1=v))
#define seteqtype(a,v)          (((a)==hashtable[(a)%HASHTABLESIZE].p) ? \
                                 (hashtable[(a)%HASHTABLESIZE].mw.c4p_P2.hh.c4p_P1.c4p_P0.b0=v) : \
                                 (createeqtbpos(a)->mw.c4p_P2.hh.c4p_P1.c4p_P0.b0=v))

#define newhashnext(a)          (((a)==hashtable[(a)%HASHTABLESIZE].p) ? \
                                 (hashtable[(a)%HASHTABLESIZE].mw.c4p_P2.hh.c4p_P1.lh) : \
                                 (createhashpos(a)->mw.c4p_P2.hh.c4p_P1.lh))
#define newhashtext(a)          (((a)==hashtable[(a)%HASHTABLESIZE].p) ? \
                                 (hashtable[(a)%HASHTABLESIZE].mw.c4p_P2.hh.rh) : \
                                 (createhashpos(a)->mw.c4p_P2.hh.rh))
#define sethashnext(a,d)        (((a)==hashtable[(a)%HASHTABLESIZE].p) ? \
                                 (hashtable[(a)%HASHTABLESIZE].mw.c4p_P2.hh.c4p_P1.lh=d) : \
                                 (createhashpos(a)->mw.c4p_P2.hh.c4p_P1.lh=d))
#define sethashtext(a,d)        (((a)==hashtable[(a)%HASHTABLESIZE].p) ? \
                                 (hashtable[(a)%HASHTABLESIZE].mw.c4p_P2.hh.rh=d) : \
                                 (createhashpos(a)->mw.c4p_P2.hh.rh=d))

#define newxeqlevel(a)          (((a)==hashtable[(a)%HASHTABLESIZE].p) ? \
                                 (hashtable[(a)%HASHTABLESIZE].mw.cint) : \
                                 (createxeqlevel(a)->mw.cint))

#define setintzero(w,a)         ((w).cint=(a))
#define setintone(w,a)          ((w).cint1=(a))
