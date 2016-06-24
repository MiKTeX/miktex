/* runtexlua.cpp: run a texlua script

   Copyright (C) 2010-2016 Christian Schenk

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
#include <miktex/Core/Cfg>
#include <miktex/Core/BufferSizes>
#include <miktex/Core/Exceptions>
#include <miktex/Core/Paths>

using namespace MiKTeX::App;
using namespace MiKTeX::Core;
using namespace std;

#define TU_(x) MiKTeX::Util::CharBuffer<char>(x).GetData()
#define UT_(x) MiKTeX::Util::CharBuffer<wchar_t>(x).GetData()

extern "C" int MIKTEXCEECALL Main(int argc, char ** argv);

// Keep the application object in the global scope (C functions might
// call exit())
MiKTeX::App::Application app;

#if MTXRUN
#  define CFGKEY "mtxrun"
#else
#  define CFGKEY "texlua"
#endif

#if defined(_UNICODE)
#  define MAIN wmain
#  define MAINCHAR wchar_t
#else
#  define MAIN main
#  define MAINCHAR char
#endif

int MAIN(int argc, const MAINCHAR ** argv)
{
  try
    {
      app.Init (TU_(argv[0]));

      MIKTEX_ASSERT (argc > 0);

      // determine script name
      char szProgramName[BufferSizes::MaxPath];
      PathName::Split (TU_(argv[0]),
		       0, 0,
		       szProgramName, BufferSizes::MaxPath,
		       0, 0);

      std::string scriptName;

#if MTXRUN
      bool isLuatools = (PathName::Compare(szProgramName, "luatools") == 0);
      bool isMtxrun = (PathName::Compare(szProgramName, "mtxrun") == 0);
      bool isTexmfstart = (PathName::Compare(szProgramName, "texmfstart") == 0);
      if (isLuatools)
      {
	scriptName = "luatools";
      }
      else
      {
	scriptName = "mtxrun";
      }
#else
      scriptName = szProgramName;
#endif

      // get relative script path
      PathName scriptsIni = app.GetSession()->GetSpecialPath(SpecialPath::DistRoot);
      scriptsIni /= MIKTEX_PATH_SCRIPTS_INI;
      unique_ptr<Cfg> pConfig (Cfg::Create());
      pConfig->Read (scriptsIni, true);
      std::string relScriptPath;
      if (! pConfig->TryGetValue(CFGKEY, scriptName.c_str(), relScriptPath))
      {
	MIKTEX_FATAL_ERROR_2 (MIKTEXTEXT("The Lua script is not registered."), szProgramName);
      }
      pConfig = nullptr;
      
      // find script
      PathName scriptPath;
      if (! app.GetSession()->FindFile(relScriptPath.c_str(), MIKTEX_PATH_TEXMF_PLACEHOLDER, scriptPath))
      {
        MIKTEX_FATAL_ERROR_2(MIKTEXTEXT("The Lua script could not be found."), relScriptPath);
      }
      
      // build new argv
      vector<string> utf8args;
      utf8args.reserve (argc + 3);
      utf8args.push_back (TU_(argv[0]));
      utf8args.push_back ("--luaonly");
      utf8args.push_back (scriptPath.Get());
#if MTXRUN
      if (! (isLuatools || isMtxrun || isTexmfstart))
      {
	utf8args.push_back ("--script");
	utf8args.push_back (szProgramName);
      }
#endif
      for (int idx = 1; idx < argc; ++ idx)
      {
	utf8args.push_back (TU_(argv[idx]));
      }
      vector<char *> newargv;
      newargv.reserve (utf8args.size() + 1);
      for (vector<string>::const_iterator it = utf8args.begin(); it != utf8args.end(); ++ it)
      {
	newargv.push_back (const_cast<char *>(it->c_str()));
      }
      newargv.push_back (0);

      // run texlua
      int exitCode = Main(newargv.size() - 1, &newargv[0]);

      app.Finalize ();

      return (exitCode);
    }
  catch (const MiKTeXException & e)
    {
      Utils::PrintException (e);
      return (1);
    }
  catch (const std::exception & e)
    {
      Utils::PrintException (e);
      return (1);
    }
  catch (int exitCode)
    {
      return (exitCode);
    }
}
