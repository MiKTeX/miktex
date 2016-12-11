/* md5walk.cpp: calculate the MD5 of a file tree

   Copyright (C) 2001-2016 Christian Schenk

   This file is part of MD5Walk.

   MD5Walk is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.
   
   MD5Walk is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with MD5Walk; if not, write to the Free Software Foundation,
   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. */

#include <cstdarg>
#include <cstdio>
#include <cstdlib>

#if defined(HAVE_CONFIG_H)
#  include <config.h>
#endif

#include <miktex/Core/Directory>
#include <miktex/Core/DirectoryLister>
#include <miktex/Core/Exceptions>
#include <miktex/Core/File>
#include <miktex/Core/IntegerTypes>
#include <miktex/Core/MD5>
#include <miktex/Core/Session>
#include <miktex/Core/less_icase_dos>
#include <miktex/Util/StringUtil>
#include <miktex/Wrappers/PoptWrapper>
#include "md5walk-version.h"

#if defined(MIKTEX_WINDOWS) && defined(_MSC_VER)
#include <io.h>
#include <conio.h>
#include <fcntl.h>
#include <windows.h>
#endif

#include <future>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <string>

#include <unordered_map>

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;
using namespace MiKTeX::Wrappers;
using namespace std;

#define T_(x) MIKTEXTEXT(x)

const char * MD5WALK_FILE = ".nvsbl";
const char * NAME_CHECK_INTEGRITY = "chkdata";

inline long MakeLong(unsigned a, unsigned b)
{
  unsigned long ul = (a & 0xffff);
  ul |= static_cast<unsigned long>(b & 0xffff) << 16;
  return static_cast<long>(ul);
}

inline unsigned MakeWord(unsigned char a, unsigned char b)
{
  unsigned u = (a & 0xff);
  u |= static_cast<unsigned>(b & 0xff) << 8;
  return static_cast<long>(u);
}

class hash_compare_md5sum
{
public:
  size_t operator() (const MD5 & md5) const
  {
    long b03 = MakeLong(MakeWord(md5[0], md5[1]), MakeWord(md5[2], md5[3]));
#if SIZE_MAX == 0xffffffff
    return b03;
#else
    long b47 = MakeLong(MakeWord(md5[4], md5[5]), MakeWord(md5[6], md5[7]));
    return (static_cast<uint64_t>(b47) << 32) | b03;
#endif
  }
  
public:
  bool operator() (const MD5 & md5_1, const MD5 & md5_2) const
  {
    return md5_1 < md5_2;
  }
};

enum TASK { None, Check, ComputeDigest, FindDuplicates, List };

const int SUCCESS = 0;
const int FATAL_ERROR = 1;
const int CANNOT_CHECK_INTEGRITY = 2;
const int NOT_OF_INTEGRITY = 3;

typedef map<string, future<MD5>, less_icase_dos> FileNameToMD5;
typedef multimap<size_t, string> SizeToFileName;
typedef unordered_multimap<MD5, string, hash_compare_md5sum, hash_compare_md5sum> MD5ToFileName;

set<string> ignoreExtensions;
int optVerbose = false;
int optBinary = false;
int optAsync = false;
#if defined(MIKTEX_WINDOWS)
int optPauseWhenFinished = false;
#endif

void Verbose(const char * lpszFormat, ...)
{
  if (!optVerbose)
  {
    return;
  }
  va_list arglist;
  va_start(arglist, lpszFormat);
  cout << StringUtil::FormatStringVA(lpszFormat, arglist);
  va_end(arglist);
}

enum Option
{
  OPT_AAA = 1000,
  OPT_CHECK,
  OPT_COMPUTE_DIGEST,
  OPT_EXCLUDE,
  OPT_FIND_DUPLICATES,
  OPT_LIST,
  OPT_VERSION,
};

const struct poptOption aoption[] = {
  {
    "async", 0, POPT_ARG_VAL, &optAsync, true,
    T_("Calculate MD5 sums asynchronously."),
    nullptr
  },
  {
    "binary", 0, POPT_ARG_VAL, &optBinary, true,
    T_("Print binary MD5."), nullptr
  },
  {
    "check", 0, POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr, OPT_CHECK,
    T_("Check the data integrity."),
    nullptr
  },
  {
    "compute-digest", 0, POPT_ARG_NONE, nullptr, OPT_COMPUTE_DIGEST,
    T_("Compute the MD5."),
    nullptr
  },
  {
    "exclude", 0, POPT_ARG_STRING, nullptr, OPT_EXCLUDE,
    T_("Files (*.EXT) to be excluded."),
    T_(".EXT")
  },
  {
    T_("find-duplicates"), 0, POPT_ARG_NONE, nullptr, OPT_FIND_DUPLICATES,
    T_("Find duplicates."),
    nullptr
  },
  {
    T_("list"), 0, POPT_ARG_NONE, nullptr, OPT_LIST,
    T_("List MD5 hashes."),
    nullptr
  },
  {
    "verbose", 0, POPT_ARG_VAL, &optVerbose, true,
    T_("Print information about what is being done."), nullptr
  },
  {
    "version", 0, POPT_ARG_NONE, nullptr, OPT_VERSION,
    T_("Show version information and exit."), nullptr
  },
  POPT_AUTOHELP
  POPT_TABLEEND
};

void FatalError(const char * lpszFormat, ...)
{
  va_list arglist;
  va_start(arglist, lpszFormat);
  cerr
    << Utils::GetExeName() << ": "
    << StringUtil::FormatStringVA(lpszFormat, arglist)
    << endl;
  va_end(arglist);
  throw FATAL_ERROR;
}

void DirectoryWalk(TASK task, const PathName & path, const string & stripPrefix, FileNameToMD5 & mapFnToMD5, SizeToFileName & mapSizeToFn)
{
  unique_ptr<DirectoryLister> lister = DirectoryLister::Open(path);
  DirectoryEntry2 dirEntry;
  while (lister->GetNext(dirEntry))
  {
    PathName path2(path);
    path2 /= dirEntry.name;
    if (dirEntry.isDirectory)
    {
      // RECURSION
      DirectoryWalk(task, path2, stripPrefix, mapFnToMD5, mapSizeToFn);
    }
    else
    {
      string ext = path2.GetExtension();
      if (ext.empty() || ignoreExtensions.find(ext) == ignoreExtensions.end())
      {
        switch (task)
        {
        case Check:
        case ComputeDigest:
        case List:
        {
          launch launchPolicy = optAsync ? launch::async : launch::deferred;
          mapFnToMD5[Utils::GetRelativizedPath(path2.GetData(), stripPrefix.c_str())] = async(launchPolicy, [](const PathName & path2) { return MD5::FromFile(path2.GetData()); }, path2);
          break;
        }
        case FindDuplicates:
          mapSizeToFn.insert(make_pair(dirEntry.size, path2.GetData()));
          break;
        default:
          break;
        }
      }
    }
  }
  lister->Close();
}

void PrintMD5(const MD5 & md5)
{
  if (optBinary)
  {
#if defined(MIKTEX_WINDOWS) && defined(_MSC_VER)
    _setmode(_fileno(stdout), _O_BINARY);
#endif
    fwrite(&md5[0], md5.size(), 1, stdout);
  }
  else
  {
    cout << md5.ToString() << endl;
  }
}

void PrintDuplicates(const set<string> & setstr)
{
  if (setstr.size() <= 1)
  {
    return;
  }
  Verbose(T_("found %u identical files (size: %u):\n"),
    static_cast<unsigned>(setstr.size()),
    static_cast<unsigned>(File::GetSize(setstr.begin()->c_str())));
  for (const string & s : setstr)
  {
    cout << "  " << s << endl;
  }
  cout << endl;
}

void FindDuplicateFiles(const set<string> & setstr)
{
  if (setstr.size() <= 1)
  {
    return;
  }
  MD5ToFileName mapMd5sumToFn;
  for (const string & s : setstr)
  {
    mapMd5sumToFn.insert(make_pair(MD5::FromFile(s), s));
  }
  MD5 md5Last;
  set<string> setstrFiles;
  for (MD5ToFileName::const_iterator it = mapMd5sumToFn.begin(); it != mapMd5sumToFn.end(); ++it)
  {
    if (it != mapMd5sumToFn.begin() && !(md5Last == it->first))
    {
      PrintDuplicates(setstrFiles);
      setstrFiles.clear();
    }
    setstrFiles.insert(it->second);
    md5Last = it->first;
  }
  PrintDuplicates(setstrFiles);
}

void Main(int argc, const char ** argv)
{
  int option;

  PoptWrapper popt(argc, argv, aoption);

  popt.SetOtherOptionHelp(T_("[OPTION...] [DIRECTORY...]"));

  TASK task;

  if (PathName::Compare(Utils::GetExeName().c_str(), NAME_CHECK_INTEGRITY) == 0)
  {
    task = Check;
    optVerbose = true;
#if defined(MIKTEX_WINDOWS)
    optPauseWhenFinished = true;
#endif
  }
  else
  {
    task = ComputeDigest;
  }

  ignoreExtensions.insert(MD5WALK_FILE);

  while ((option = popt.GetNextOpt()) >= 0)
  {
    switch (option)
    {
    case OPT_CHECK:
      task = Check;
      break;
    case OPT_COMPUTE_DIGEST:
      task = ComputeDigest;
      break;
    case OPT_EXCLUDE:
      ignoreExtensions.insert(popt.GetOptArg());
      break;
    case OPT_FIND_DUPLICATES:
      task = FindDuplicates;
      break;
    case OPT_LIST:
      task = List;
      break;
    case OPT_VERSION:
      cout
        << Utils::MakeProgramVersionString(Utils::GetExeName(), VersionNumber(MIKTEX_MAJOR_VERSION, MIKTEX_MINOR_VERSION, MIKTEX_COMP_J2000_VERSION, 0)) << endl
        << "Copyright (C) 2005-2016 Christian Schenk" << endl
        << "This is free software; see the source for copying conditions.  There is NO" << endl
        << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE." << endl;
      return;
    }
  }

  if (option != -1)
  {
    string msg = popt.BadOption(POPT_BADOPTION_NOALIAS);
    msg += ": ";
    msg += popt.Strerror(option);
    FatalError("%s", msg.c_str());
  }

  FileNameToMD5 mapFnToMD5;
  SizeToFileName mapSizeToFn;

  vector<string> directories;

  vector<string> leftovers = popt.GetLeftovers();

  if (leftovers.empty())
  {
    if (task == Check)
    {
      directories.push_back(Directory::GetCurrent().ToString());
    }
  }
  else
  {
    for (const string & dir : leftovers)
    {
      directories.push_back(dir);
    }
  }

  MD5 md5Good;
  bool haveMD5File = false;

  for (const string & dir : directories)
  {
    if (task == Check)
    {
      Verbose(T_("Checking the data integrity of \"%s\"...\n"), dir.c_str());
    }
    DirectoryWalk(task, dir, dir, mapFnToMD5, mapSizeToFn);
    if (task == Check && !haveMD5File)
    {
      PathName md5File = dir;
      md5File /= MD5WALK_FILE;
      if (File::Exists(md5File))
      {
        haveMD5File = true;
        vector<unsigned char> bytes = File::ReadAllBytes(md5File);
        if (bytes.size() != md5Good.size())
        {
          MIKTEX_UNEXPECTED();
        }
        std::copy(bytes.begin(), bytes.end(), md5Good.begin());
      }
    }
  }

  switch (task)
  {
  case Check:
  case ComputeDigest:
  {
    MD5Builder md5Builder;
    for (auto & p : mapFnToMD5)
    {
      PathName path(p.first);
      // we must dosify the path name for backward compatibility
      path.ConvertToDos();
      md5Builder.Update(path.GetData(), path.GetLength());
      md5Builder.Update(&p.second.get()[0], p.second.get().size());
    }
    MD5 md5 = md5Builder.Final();
    if (task == Check)
    {
      Verbose(T_("Done.\nFindings: "));
      if (!haveMD5File)
      {
        cerr << T_("The data might have been corrupted.") << endl;
        throw CANNOT_CHECK_INTEGRITY;
      }
      if (md5 == md5Good)
      {
        Verbose(T_("The data is intact.\n"));
      }
      else
      {
        cerr << T_("The data has been corrupted.") << endl;
        throw NOT_OF_INTEGRITY;
      }
    }
    else
    {
      PrintMD5(md5);
    }
    break;
  }
  case FindDuplicates:
  {
    set<string> setstr;
    size_t lastSize = 0;
    for (SizeToFileName::const_iterator it = mapSizeToFn.begin(); it != mapSizeToFn.end(); ++it)
    {
      if (it != mapSizeToFn.begin() && it->first != lastSize)
      {
        FindDuplicateFiles(setstr);
        setstr.clear();
      }
      setstr.insert(it->second);
      lastSize = it->first;
    }
    FindDuplicateFiles(setstr);
    break;
  }
  case List:
    for (auto & p : mapFnToMD5)
    {
      cout << p.second.get().ToString() << " " << PathName(p.first).ToUnix() << endl;
    }
  default:
    break;
  }

}

int main(int argc, const char ** argv)
{
  int exitCode;
  try
  {
    shared_ptr<Session> session = Session::Create(Session::InitInfo(argv[0]));
    Main(argc, argv);
    exitCode = 0;
  }
  catch (const MiKTeXException & e)
  {
    Utils::PrintException(e);
    exitCode = FATAL_ERROR;
  }
  catch (const exception & e)
  {
    Utils::PrintException(e);
    exitCode = FATAL_ERROR;
  }
  catch (int r)
  {
    exitCode = r;
  }
#if defined(MIKTEX_WINDOWS)
  if (optPauseWhenFinished)
  {
    cout << endl << T_("Press any key to continue...") << flush;
    _getch();
  }
#endif
  return exitCode;
}
