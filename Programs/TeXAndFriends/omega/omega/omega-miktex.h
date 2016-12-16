/* omega-miktex.h:                                      -*- C++ -*-

   Copyright (C) 1998-2016 Christian Schenk

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

#if defined(MIKTEX)
#define P1H(x1) (x1)
#define P1C(t1, v1) (t1 v1)
#define P2C(t1, v1, t2, v2) (t1 v1, t2 v2)
#define P4C(t1, v1, t2, v2, t3, v3, t4, v4) (t1 v1, t2 v2, t3 v3, t4 v4)
#endif

#if defined(MIKTEX_OMEGA)
#  include "omegadefs.h"
#endif

#if ! defined(C4PEXTERN)
#  define C4PEXTERN extern
#endif

#if defined(MIKTEX_OMEGA)
#  include "omega.h"
#endif

#if !defined(THEDATA)
#  define THEDATA(x) C4P_VAR(x)
#endif

#if defined(MIKTEX_OMEGA) && defined(MIKTEX_WINDOWS)
#  include "omega.rc"
#endif

namespace omega {
#include <miktex/omega.defaults.h>
}

#include <miktex/TeXAndFriends/TeXApp>

#if !defined(MIKTEX_VIRTUAL_TEXAPP)
#  define MIKTEX_VIRTUAL_TEXAPP
#endif

#if !defined(MIKTEXHELP_OMEGA)
#  include <miktex/Core/Help>
#endif

using namespace MiKTeX::TeXAndFriends;
using namespace MiKTeX::Core;

#define uexit(exitCode) throw(exitCode)

class OMEGACLASS :
  MIKTEX_VIRTUAL_TEXAPP public TeXApp
{
protected:
  void AddOptions() override
  {
    TeXApp::AddOptions();
    AddOption("oft", "undump");
  }

public:
  void AllocateMemory()
  {
    TeXApp::AllocateMemory();
    GETPARAM(-1, ocpbufsize, ocp_buf_size, omega::omega::ocp_buf_size());
    GETPARAM(-1, ocplistinfosize, ocp_listinfo_size, omega::omega::ocp_listinfo_size());
    GETPARAM(-1, ocplistlistsize, ocp_list_list_size, omega::omega::ocp_list_list_size());
    GETPARAM(-1, ocplstacksize, ocp_lstack_size, omega::omega::ocp_lstack_size());
    GETPARAM(-1, ocpstacksize, ocp_stack_size, omega::omega::ocp_stack_size());
    Allocate("inputfilemode", THEDATA(inputfilemode), THEDATA(maxinopen));
    Allocate("inputfiletranslation", THEDATA(inputfiletranslation), THEDATA(maxinopen));
    Allocate("ocplistinfo", THEDATA(ocplistinfo), THEDATA(ocplistinfosize));
    Allocate("ocplistlist", THEDATA(ocplistlist), THEDATA(ocplistlistsize));
    Allocate("ocplstackinfo", THEDATA(ocplstackinfo), THEDATA(ocplstacksize));
    Allocate("strstartar", THEDATA(strstartar), THEDATA(maxstrings));
    Allocate("triec", THEDATA(triec), THEDATA(triesize));
    Allocate("trieoplang", THEDATA(trieoplang), THEDATA(trieopsize));
  }

public:
  void FreeMemory()
  {
    TeXApp::FreeMemory();
    Free(THEDATA(inputfilemode));
    Free(THEDATA(inputfiletranslation));
    Free(THEDATA(ocplistinfo));
    Free(THEDATA(ocplistlist));
    Free(THEDATA(ocplstackinfo));
    Free(THEDATA(strstartar));
    Free(THEDATA(triec));
    Free(THEDATA(trieoplang));
  }

private:
  std::shared_ptr<MiKTeX::Core::Session> session;

public:
  void Init(const char * lpszProgramInvocationName) override
  {
    TeXApp::Init(lpszProgramInvocationName);
    session = GetSession();
#if defined(IMPLEMENT_TCX)
    EnableFeature(Feature::TCX);
#endif
  }

public:
  const char * GetMemoryDumpFileName() const override
  {
    return "omega.fmt";
  }

public:
  const char * GetPoolFileName() const
  {
    return "omega.pool";
  }

public:
  const char * GetInitProgramName() const override
  {
    return "iniomega";
  }

public:
  const char * GetVirginProgramName() const override
  {
    return "viromega";
  }

public:
  const char * TheNameOfTheGame() const override
  {
    return "Omega";
  }

public:
  unsigned long GetHelpId() const override
  {
    return MIKTEXHELP_OMEGA;
  }

public:
  bool OpenOCPFile(bytefile & f, char * lpszFileName)
  {
    PathName fileName;
    if (!session->FindFile(lpszFileName, FileType::OCP, fileName))
    {
      return false;
    }
    FILE * pfile = session->TryOpenFile(fileName.GetData(), FileMode::Open, FileAccess::Read, false);
    if (pfile == nullptr)
    {
      return false;
    }
    f.Attach(pfile, true);
    get(f);
    return true;
  }
  
public:
  bool OpenONMFile(alphafile & f, char * lpszFileName)
  {
#if 1
    return false;
#else
    PathName fileName;
    if (!session->FindFile(lpszFileName, FileType::ONM, fileName))
    {
      return false;
    }
    FILE * pfile = sessionOpenFile(fileName.Get(), FileMode::Open, FileAccess::Read, false);
    if (pfile == nullptr)
    {
      return false;
    }
    f.Attach(pfile, true);
#if defined(PASCAL_TEXT_IO)
    get(f);
#endif
    return true;
#endif
  }
};

#if defined(MIKTEX_OMEGA)
extern OMEGACLASS OMEGAAPP;
#define THEAPP OMEGAAPP
#include <miktex/TeXAndFriends/TeXApp.inl>
int miktexloadpoolstrings(int size);

inline int loadpoolstrings(int size)
{
  return miktexloadpoolstrings(size);
}
#endif

inline bool miktexopenocpfile(bytefile & f, char * n)
{
  return THEAPP.OpenOCPFile(f, n);
}

inline bool miktexopenonmfile(alphafile & f, char * n)
{
  return THEAPP.OpenONMFile(f, n);
}

#define first THEDATA(first)
#define fmtfile THEDATA(fmtfile)
#define fontsorttables THEDATA(fontsorttables)
#define fonttables THEDATA(fonttables)
#define last THEDATA(last)
#define namelength THEDATA(namelength)
#define nameoffile (&(THEDATA(nameoffile)[-1]))
#define ocpbufsize THEDATA(ocpbufsize)
#define ocptables THEDATA(ocptables)
#define otpinputbuf THEDATA(otpinputbuf)
#define otpinputend THEDATA(otpinputend)
#define otpoutputbuf THEDATA(otpoutputbuf)
#define otpoutputend THEDATA(otpoutputend)

#include <miktex/KPSE/Emulation>
#include <miktex/W2C/Emulation>

#define cint c4p_P2.c4p_int
#define cint1 c4p_P2.hh.c4p_P1.lh
#define dumpwd(wd) *fmtfile=wd; put(fmtfile)
#define dumpint(i) (*fmtfile).cint=i; put(fmtfile)
#define undumpwd(wd) get(fmtfile); wd=*fmtfile
#define undumpint(i) get(fmtfile); i=(*fmtfile).cint
#define dumpthings(first_item, n) fwrite(&first_item, sizeof (first_item), n, fmtfile)
#define undumpthings(first_item, n) fread(&first_item, sizeof (first_item), n, fmtfile)
#define ziniteqtbentry initeqtbentry

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
