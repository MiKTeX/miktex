/* rungs.cpp: the MiKTeX Ghostscript runner

   Copyright (C) 2019 Christian Schenk

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
   USA.  */

#include <miktex/App/Application>
#include <miktex/Core/Exceptions>
#include <miktex/Util/StringUtil>

#include <string>
#include <vector>

using namespace MiKTeX::App;
using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

using namespace std;

#if defined(_UNICODE)
int wmain(int argc, wchar_t** argv)
#else
int main(int argc, char** argv)
#endif
{
  string programName = "rungs";
  Application app;
  try
  {
    vector<string> utf8args;
    utf8args.reserve(argc);
    vector<char*> newargv;
    newargv.reserve(argc + 1);
    for (int idx = 0; idx < argc; ++idx)
    {
#if defined(_UNICODE)
      utf8args.push_back(StringUtil::WideCharToUTF8(argv[idx]));
#elif defined(MIKTEX_WINDOWS)
      utf8args.push_back(StringUtil::AnsiToUTF8(argv[idx]));
#else
      utf8args.push_back(argv[idx]);
#endif
      // FIXME: eliminate const cast
      newargv.push_back(const_cast<char*>(utf8args[idx].c_str()));
    }
    newargv.push_back(nullptr);
    programName = PathName(newargv[0]).GetFileName().ToString();
    app.Init(newargv);
    int exitCode = app.GetSession()->RunGhostscript(newargv.size() - 1, const_cast<const char**>(&newargv[0]));
    app.Finalize2(exitCode);
    return exitCode;
  }
  catch (const MiKTeXException& e)
  {
    Application::Sorry(programName, e);
    e.Save();
    return 1;
  }
  catch (const std::exception& e)
  {
    Application::Sorry(programName, e);
    return 1;
  }
}
