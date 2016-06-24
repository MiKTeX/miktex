/* cfg.cpp:

   Copyright (C) 2006-2016 Christian Schenk

   This file is part of cfg.

   cfg is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   cfg is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with cfg; if not, write to the Free Software Foundation, 59
   Temple Place - Suite 330, Boston, MA 02111-1307, USA. */

#include <cstdarg>
#include <cstdio>
#include <cstdlib>

#if defined(_MSC_VER) || defined(__MINGW32__)
#  include <conio.h>
#endif

#include <miktex/Core/Cfg>
#include <miktex/Core/Exceptions>
#include <miktex/Core/MD5>
#include <miktex/Core/Session>
#include <miktex/Util/StringUtil>
#include <miktex/Wrappers/PoptWrapper>

#include "cfg-version.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <locale>

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;
using namespace MiKTeX::Wrappers;
using namespace std;

#define T_(x) MIKTEXTEXT(x)

enum TASK { None, ComputeDigest, PrintClasses, SetValue, Sign };

enum Option
{
  OPT_AAA = 1000,
  OPT_COMPUTE_DIGEST,
  OPT_PRINT_CLASSES,
  OPT_PRIVATE_KEY_FILE,
  OPT_SET_VALUE,
  OPT_SIGN,
  OPT_VERSION,
};

const struct poptOption aoption[] = {
  {
    "compute-digest", 0, POPT_ARG_NONE, nullptr, OPT_COMPUTE_DIGEST,
    T_("Compute the MD5."), nullptr,
  },
  {
    "print-classes", 0, POPT_ARG_NONE, nullptr, OPT_PRINT_CLASSES,
    T_("Print C++ class definitions."), nullptr,
  },
  {
    "private-key-file", 0, POPT_ARG_STRING, nullptr, OPT_PRIVATE_KEY_FILE,
    T_("The private key file used for signing."), T_("FILE"),
  },
  {
    "set-value", 0, POPT_ARG_STRING, nullptr, OPT_SET_VALUE,
    T_("Sets a value."), T_("NAME=VALUE"),
  },
  {
    "sign", 0, POPT_ARG_NONE, nullptr, OPT_SIGN,
    T_("Sign the cfg file."), nullptr,
  },
  {
    T_("version"), 0, POPT_ARG_NONE, nullptr, OPT_VERSION,
    T_("Show version information and exit."), nullptr
  },
  POPT_AUTOHELP
  POPT_TABLEEND
};

void FatalError(const char * lpszFormat, ...)
{
  va_list arglist;
  va_start(arglist, lpszFormat);
  cerr << Utils::GetExeName() << ": " << StringUtil::FormatString(lpszFormat, arglist) << endl;
  va_end(arglist);
  throw (1);
}

void PrintDigest(const MD5 & md5)
{
  cout << md5.ToString() << endl;
}

string ToStr(const string & s)
{
  string result;
  for (char ch : s)
  {
    switch (ch)
    {
    case '\\':
      result += "\\\\";
      break;
    case '"':
      result += "\\\"";
      break;
    default:
      result += ch;
      break;
    }
  }
  return result;
}

void DoPrintClasses(Cfg & cfg)
{
  for (const shared_ptr<Cfg::Key> & key : cfg.GetKeys())
  {
    cout << "class " << key->GetName() << " {" << endl;
    vector <shared_ptr<Cfg::Value>> values = key->GetValues();
    for (const shared_ptr<Cfg::Value> & val : values)
    {
      string value = val->GetValue();
      char * endptr = nullptr;
      strtol(value.c_str(), &endptr, 0);
      bool isNumber = endptr == nullptr || *endptr == 0;
      cout << "  public: static ";
      if (isNumber)
      {
        cout << "int";
      }
      else
      {        
        cout << "std::string";
      }
      cout
        << " " << val->GetName()
        << "() { return ";
      if (isNumber)
      {
        cout << value;
      }
      else
      {
        cout << '"' << ToStr(value) << '"';
      }
      cout << "; }" << endl;
    }
    cout << "};" << endl;
  }
}

class PrivateKeyProvider : public IPrivateKeyProvider
{
public:
  PrivateKeyProvider(const PathName & privateKeyFile) :
    privateKeyFile(privateKeyFile)
  {
  }
public:
  PathName MIKTEXTHISCALL GetPrivateKeyFile() override
  {
    return privateKeyFile;
  }
public:
  bool GetPassphrase(std::string & passphrase) override
  {
#if defined(MIKTEX_WINDOWS)
    fputs(T_("Passphrase: "), stdout);
    const int EOL = '\r';
    passphrase = "";
    int ch;
    while ((ch = getch()) != EOL)
    {
      passphrase += ch;
    }
    putchar('\n');
    return true;
#else
    // TODO: read password from stdin
    cerr << "Unimplemented: read password from stdin" << endl;
    return false;
#endif
  }
private:
  PathName privateKeyFile;
};

void Main(int argc, const char ** argv)
{
  int option;

  PoptWrapper popt(argc, argv, aoption);

  popt.SetOtherOptionHelp(T_("[OPTION...] CFGFILE..."));

  TASK task = ComputeDigest;

  PathName privateKeyFile;
  vector<pair<string, string>> values;

  while ((option = popt.GetNextOpt()) >= 0)
  {
    string optArg = popt.GetOptArg();
    switch (option)
    {
    case OPT_COMPUTE_DIGEST:
      task = ComputeDigest;
      break;
    case OPT_PRINT_CLASSES:
      task = PrintClasses;
      break;
    case OPT_PRIVATE_KEY_FILE:
      privateKeyFile = optArg;
      break;
    case OPT_SET_VALUE:
    {
      task = SetValue;
      size_t pos = optArg.find('=');
      if (pos == string::npos)
      {
        FatalError("bad value");
      }
      values.push_back(make_pair<string, string>(optArg.substr(0, pos), optArg.substr(pos + 1)));
      break;
    }
    case OPT_SIGN:
      task = Sign;
      break;
    case OPT_VERSION:
      cout
        << Utils::MakeProgramVersionString(Utils::GetExeName().c_str(), VersionNumber(MIKTEX_MAJOR_VERSION, MIKTEX_MINOR_VERSION, MIKTEX_COMP_J2000_VERSION, 0)) << endl
        << T_("Copyright (C) 2006-2016 Christian Schenk") << endl
        << T_("This is free software; see the source for copying conditions.  There is NO") << endl
        << T_("warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.") << endl;
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

  vector<string> leftovers = popt.GetLeftovers();

  if (leftovers.empty())
  {
    FatalError(T_("no file name arguments"));
  }

  for (const string & fileName : leftovers)
  {
    unique_ptr<Cfg> pCfg(Cfg::Create());
    pCfg->Read(fileName);
    if (task == ComputeDigest)
    {
      PrintDigest(pCfg->GetDigest());
    }
    else if (task == PrintClasses)
    {
      DoPrintClasses(*pCfg);
    }
    else if (task == Sign)
    {
      PrivateKeyProvider privateKeyProvider(privateKeyFile);
      pCfg->Write(fileName, "", &privateKeyProvider);
    }
    else if (task == SetValue)
    {
      for (const pair<string, string> & nv : values)
      {
        pCfg->PutValue("", nv.first, nv.second);
      }
      pCfg->Write(fileName, "");
    }
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
    exitCode = 1;
  }
  catch (const exception & e)
  {
    Utils::PrintException(e);
    exitCode = 1;
  }
  catch (int r)
  {
    exitCode = r;
  }
  return exitCode;
}
