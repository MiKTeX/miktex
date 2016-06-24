/* dviscan.cpp: test driver for the DVI interfaces

   Copyright (C) 1996-2016 Christian Schenk

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

#include <cstdarg>
#include <cstdlib>

#include <iostream>

#include <miktex/App/Application>
#include <miktex/Core/Text>
#include <miktex/DVI/Dvi>
#include <miktex/Util/StringUtil>
#include <miktex/Wrappers/PoptWrapper>

using namespace MiKTeX::App;
using namespace MiKTeX::Core;
using namespace MiKTeX::DVI;
using namespace MiKTeX::Util;
using namespace MiKTeX::Wrappers;
using namespace std;

#define T_(x) MIKTEXTEXT(x)

enum {
  OPT_AAA = 1234,
  OPT_TRACE,
  OPT_PAGE_MODE
};

static const struct poptOption long_options[] = {
  {
    "page-mode", 0, POPT_ARG_STRING, nullptr, OPT_PAGE_MODE, "Sets the DVI page mode.", "PAGEMODE"
  },
  {
    "trace", 0, POPT_ARG_STRING, nullptr, OPT_TRACE, "Turn on tracing.", "TRACESTREAMS"
  },
  POPT_AUTOHELP
  POPT_TABLEEND
};

class DviScanner :
  public Application
{
public:
  void Run(int argc, const char ** argv);

private:
  DviPageMode pageMode = DviPageMode::Pk;
};

void DviScanner::Run(int argc, const char ** argv)
{
  Session::InitInfo initInfo(argv[0]);

  string metafontMode("ljfour");
  int dpi = 600;

  PoptWrapper popt(argc, argv, long_options);

  popt.SetOtherOptionHelp(T_("[OPTION...] DVIFILE..."));

  int option;

  while ((option = popt.GetNextOpt()) >= 0)
  {
    string optArg = popt.GetOptArg();
    switch (option)
    {
    case OPT_PAGE_MODE:
      if (optArg == "pk")
      {
        pageMode = DviPageMode::Pk;
      }
      else if (optArg == "dvips")
      {
        pageMode = DviPageMode::Dvips;
      }
      else
      {
        throw T_("invalid page mode");
      }
      break;
    case OPT_TRACE:
      initInfo.SetTraceFlags(optArg);
      break;
    }
  }

  if (option < -1)
  {
    cerr << popt.BadOption(POPT_BADOPTION_NOALIAS) << ": " << popt.Strerror(option) << endl;
    throw 1;
  }

  vector<string> leftovers = popt.GetLeftovers();

  if (leftovers.empty())
  {
    cerr << "missing file name argument" << endl;
    throw 1;
  }

  Init(initInfo);

  for (const string & dviFileName : leftovers)
  {
    unique_ptr<Dvi> pDvi(Dvi::Create(dviFileName.c_str(), metafontMode.c_str(), dpi, 5, DviAccess::Sequential, pageMode, session->GetPaperSizeInfo("A4size"), false, 0));
    pDvi->Scan();
    for (int i = 0; i < pDvi->GetNumberOfPages(); ++i)
    {
      DviPage * pPage = pDvi->GetLoadedPage(i);
      if (pPage == nullptr)
      {
        break;
      }
      for (int j = 0; j < pPage->GetNumberOfDviBitmaps(5); ++j)
      {
        pPage->GetDviBitmap(5, j);
      }
      pPage->Unlock();
    }
    pDvi->Dispose();
    pDvi = nullptr;
  }

  Finalize();
}

int main(int argc, const char ** argv)

{
  try
  {
    DviScanner app;
    app.Run(argc, argv);
    return 0;
  }
  catch (const MiKTeXException & e)
  {
    Utils::PrintException(e);
    return 1;
  }
  catch (const exception & e)
  {
    Utils::PrintException(e);
    return 1;
  }
  catch (const char * lpszMessage)
  {
    cerr << "fatal error: " << lpszMessage << endl;
    return 1;
  }
  catch (int retCode)
  {
    return retCode;
  }
}
