/* cjklatex.cpp: call latex after preprocessing of the .tex file by
   the cjk conversion tool

   Written in the years 2004-2016 by Christian Schenk.

   This file is based on public domain work (cjklatex.c, 2001) by
   Fabrice Popineau.

   To the extent possible under law, the author(s) have dedicated all
   copyright and related and neighboring rights to this file to the
   public domain worldwide.  This file is distributed without any
   warranty.  You should have received a copy of the CC0 Public Domain
   Dedication along with this file.  If not, see
   http://creativecommons.org/publicdomain/zero/1.0/. */

#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <climits>
#include <cstdarg>
#include <cstdio>

#include <getopt.h>

#include "cjklatex-version.h"

#include <miktex/App/Application>
#include <miktex/Core/CommandLineBuilder>
#include <miktex/Core/Exceptions>
#include <miktex/Core/FileType>
#include <miktex/Core/Paths>
#include <miktex/Core/Process>
#include <miktex/Core/Quoter>
#include <miktex/Util/StringUtil>

using namespace MiKTeX::App;
using namespace MiKTeX::Core;
using namespace MiKTeX::Util;
using namespace std;

#define T_(x) MIKTEXTEXT(x)

#define Q_(x) MiKTeX::Core::Quoter<char>(x).Get()

/* _________________________________________________________________________

   Option
   _________________________________________________________________________ */

enum Option
{
  OPT__FIRST = 1000,
  OPT_CONV,
  OPT_HELP,
  OPT_LATEX,
  OPT_NOCLEANUP,
  OPT_PRINT_ONLY,
  OPT_VERBOSE,
  OPT_VERSION,
};

/* _________________________________________________________________________

   aoption
   _________________________________________________________________________ */

const struct option aoption[] =
{
  T_("conv"),		required_argument,	0,	OPT_CONV,
  T_("latex"),		required_argument,	0,	OPT_LATEX,
  T_("help"),		no_argument,		0,	OPT_HELP,
  T_("nocleanup"),	no_argument,		0,	OPT_NOCLEANUP,
  T_("print-only"),	no_argument,		0,	OPT_PRINT_ONLY,
  T_("verbose"),	no_argument,		0,	OPT_VERBOSE,
  T_("version"),	no_argument,		0,	OPT_VERSION,
  0,			no_argument,		0,	0,
};

/* _________________________________________________________________________

   Converters
   _________________________________________________________________________ */

struct CONVERTER {
  const char *	lpszName;
  const char *	lpszProgram;
};

const CONVERTER Converters [] =
{
  "bg5",	T_("bg5conv"),
  "bg5+",	T_("extconv"),
  "bg5p",	T_("extconv"),
  T_("cef"),	T_("cefconv"),
  T_("cef5"),	T_("cef5conv"),
  T_("cefs"),	T_("cefsconv"),
  "cjk",	0,
  "gbk",	T_("extconv"),
  T_("sjis"),	T_("sjisconv"),
};

/* _________________________________________________________________________

   CJKLaTeXApp
   _________________________________________________________________________ */

class CJKLaTeXApp : public Application
{
public:
  CJKLaTeXApp ();
  
private:
  void
  Error (/*[in]*/ const char *	lpszFormat,
	 /*[in]*/		...);

private:
  void
  PrintOnly (/*[in]*/ const char *	lpszFormat,
	     /*[in]*/			...);

private:
  const char *
  GetConverterProgram (/*[in]*/ const char *	lpszName);

private:
  void
  ParseInvocationName (/*[out]*/ string &	converterProgram,
		       /*[out]*/ string &	engine);
    
private:
  void
  RunConverter (/*[in]*/ const PathName &	inputFile,
		/*[in]*/ const PathName &	intermediateFile);

private:
  void
  RunEngine (/*[in]*/ const PathName &	inputFile);

private:
  void
  ShowVersion ();
  
private:
  void
  Help ();
  
private:
  void
  ProcessOptions (/*[in]*/ int		argc,
		  /*[in]*/ char **	argv);

public:
  void
  Run (/*[in]*/ int		argc,
       /*[in]*/ char **	argv);

private:
  string engine;

private:
  string converterProgram;

private:
  bool cleanUp;

private:
  bool printOnly;
};

/* _________________________________________________________________________

   CJKLaTeXApp::CJKLaTeXApp
   _________________________________________________________________________ */

CJKLaTeXApp::CJKLaTeXApp ()
  : cleanUp (true),
    printOnly (false)
{
}

/* _________________________________________________________________________

   CJKLaTeXApp::ShowVersion
   _________________________________________________________________________ */

void
CJKLaTeXApp::ShowVersion ()
{
  cout << Utils::MakeProgramVersionString(Utils::GetExeName().c_str(),
					   MIKTEX_COMPONENT_VERSION_STR)
       << T_("\n\
Written by Fabrice Popineau in 2001.  Further developed by\n\
Christian Schenk in the years 2004-2016.\n\
\n\
To the extent possible under law, the author(s) have dedicated all\n\
copyright and related and neighboring rights to this program to the\n\
public domain worldwide.  This program is distributed without any\n\
warranty.  You should have received a copy of the CC0 Public Domain\n\
Dedication along with this file.  If not, see\n\
http://creativecommons.org/publicdomain/zero/1.0/.")
       << endl;
}

/* _________________________________________________________________________

   CJKLaTeXApp::Help
   _________________________________________________________________________ */

void
CJKLaTeXApp::Help ()
{
  cout << T_("Usage: ") << Utils::GetExeName() << T_(" [OPTION...] FILE\n\
\n\
This program runs a converter on FILE and then LaTeX on the result.\n\
\n\
Options:\n\
--conv=FILTER             Set the conversion filter. One of:\n\
                            bg5+\n\
                            bg5\n\
                            bg5p\n\
                            cef5\n\
                            cef\n\
                            cefs\n\
                            gbk\n\
                            sjis\n\
--nocleanup               Keep intermediate files.\n\
--latex=PROGRAM           Use PROGRAM instead of latex.\n\
--help                    Print this help screen and exit.\n\
--verbose                 Print info about what is being done.\n\
--version                 Print the version number and exit.")
       << endl;
}

/* _________________________________________________________________________
     
   CJKLaTeXApp::Error
   _________________________________________________________________________ */

void
CJKLaTeXApp::Error (/*[in]*/ const char *	lpszFormat,
		    /*[in]*/			...)
{
  va_list arglist;
  va_start (arglist, lpszFormat);
  cerr << Utils::GetExeName() << ": "
       << StringUtil::FormatString(lpszFormat, arglist)
       << endl;
  va_end (arglist);
  throw (1);
}

/* _________________________________________________________________________
     
   CJKLaTeXApp::PrintOnly
   _________________________________________________________________________ */

void
CJKLaTeXApp::PrintOnly (/*[in]*/ const char *	lpszFormat,
			/*[in]*/		...)
{
  if (! printOnly)
    {
      return;
    }
  va_list arglist;
  va_start (arglist, lpszFormat);
  cout << StringUtil::FormatString(lpszFormat, arglist) << endl;
  va_end (arglist);
}

/* _________________________________________________________________________

   CJKLaTeXApp::RunConverter
   _________________________________________________________________________ */

void
CJKLaTeXApp::RunConverter (/*[in]*/ const PathName &	inputFile,
			   /*[in]*/ const PathName &	intermediateFile)
{
  PathName converter;
  if (! session->FindFile(converterProgram.c_str(),
			   FileType::EXE,
			   converter))
    {
      Error (T_("Converter %s not found."), converterProgram.c_str());
    }
#if defined(MIKTEX_WINDOWS)
  Utils::RemoveBlanksFromPathName (converter);
#endif
  CommandLineBuilder cmdLine;
  cmdLine.AppendArgument (converter);
  cmdLine.AppendStdinRedirection (inputFile);
  cmdLine.AppendStdoutRedirection (intermediateFile);
  PrintOnly ("%s", cmdLine.ToString().c_str());
  if (! printOnly && ! Process::ExecuteSystemCommand(cmdLine.ToString()))
    {
      Error (T_("Converter %s failed on %s."),
	     converterProgram.c_str(),
	     Q_(inputFile));
    }
}

/* _________________________________________________________________________

   CJKLaTeXApp::RunEngine
   _________________________________________________________________________ */

void
CJKLaTeXApp::RunEngine (/*[in]*/ const PathName &	inputFile)
{
  PathName engineExe;
  if (! session->FindFile(engine.c_str(), FileType::EXE, engineExe))
    {
      Error (T_("Engine %s not found."), engine.c_str());
    }
  CommandLineBuilder arguments;
  arguments.AppendArgument (inputFile.ToString());
  PrintOnly ("%s %s", Q_(engineExe), arguments.ToString().c_str());
  if (! printOnly)
    {
      Process::Run (engineExe, arguments.ToString());
    }
}

/* _________________________________________________________________________

   CJKLaTeXApp::GetConverterProgram
   _________________________________________________________________________ */

const char *
CJKLaTeXApp::GetConverterProgram (/*[in]*/ const char *	lpszName)
{
  for (size_t i = 0; i < sizeof(Converters) / sizeof(Converters[0]); ++ i)
    {
      if (PathName::Compare(lpszName, Converters[i].lpszName) == 0)
	{
	  return (Converters[i].lpszProgram);
	}
    }
  return (0);
}

/* _________________________________________________________________________

   CJKLaTeXApp::ParseInvocationName
   _________________________________________________________________________ */

void
CJKLaTeXApp::ParseInvocationName
(/*[out]*/ string &		converterProgram,
 /*[out]*/ string &		engine)
{
  string invocationName = Utils::GetExeName();
  const char * lpszInvocationName = invocationName.c_str();
  size_t j = UINT_MAX;
  size_t len = 0;
  for (size_t i = 0; i < sizeof(Converters) / sizeof(Converters[0]); ++ i)
    {
      size_t l = strlen(Converters[i].lpszName);
#if defined(MIKTEX_WINDOWS)
      if (_strnicmp(lpszInvocationName, Converters[i].lpszName, l) == 0
#else
      if (strncmp(lpszInvocationName, Converters[i].lpszName, l) == 0
#endif
	  && l > len)
	{
	  j = i;
	  len = l;
	}
    }
  if (j == UINT_MAX)
    {
      Error (T_("Bad invocation."));
    }
  if (Converters[j].lpszProgram != 0)
    {
      converterProgram = Converters[j].lpszProgram;
    }
  engine = &lpszInvocationName[len];
}

/* _________________________________________________________________________

   CJKLaTeXApp::ProcessOptions
   _________________________________________________________________________ */

void
CJKLaTeXApp::ProcessOptions (/*[in]*/ int	argc,
			     /*[in]*/ char **	argv)
{
  ParseInvocationName (converterProgram, engine);
  
  optind = 0;
  int optionChar;
  int optionIndex;
  
  while ((optionChar = getopt_long_only(argc,
				      argv,
				      "",
				      aoption,
				      &optionIndex))
	 != EOF)
    {
      switch (static_cast<Option>(optionChar))
	{

	case OPT_CONV:

	  {
	    const char * lpszConverterProgram =
	      GetConverterProgram(optarg);
	    if (lpszConverterProgram == 0)
	      {
		Error (T_("The converter %s is unknown."), optarg);
	      }
	    converterProgram = lpszConverterProgram;
	    break;
	  }
	    
	case OPT_HELP:
	  
	  Help ();
	  throw (0);

	case OPT_LATEX:

	  engine = optarg;
	  break;

	case OPT_PRINT_ONLY:
	  
	  printOnly = true;
	  break;

	case OPT_NOCLEANUP:

	  cleanUp = false;
	  break;

	case OPT_VERBOSE:
	  
	  break;
	  
	case OPT_VERSION:
	  
	  ShowVersion ();
	  throw (0);
	  
	default:

	  Error (T_("Invalid command-line."));
	  break;
	}
    }

  EnableInstaller (TriState::True);
}

/* _________________________________________________________________________

   CJKLaTeXApp::Run
   _________________________________________________________________________ */

void
CJKLaTeXApp::Run (/*[in]*/ int		argc,
		  /*[in]*/ char **	argv)
{
  ProcessOptions (argc, argv);
  for (int i = optind; i != argc; ++ i)
    {
      PathName pathInputFile (argv[i]);
      pathInputFile.MakeAbsolute ();
      PathName pathIntermediateFile (pathInputFile);
      pathIntermediateFile.SetExtension (".cjk");
      RunConverter (pathInputFile, pathIntermediateFile);
      RunEngine (pathIntermediateFile);
      if (cleanUp && ! printOnly)
	{
	  File::Delete (pathIntermediateFile);
	}
    }
}

/* _________________________________________________________________________

   cjklatexmain
   _________________________________________________________________________ */

extern "C" MIKTEXDLLEXPORT int MIKTEXCEECALL cjklatexmain(int argc, char * argv[])
{
  try
    {
      CJKLaTeXApp app;
      app.Init (argv[0]);
      app.Run(argc, argv);
      app.Finalize ();
      return (0);
    }
  catch (const MiKTeXException & e)
    {
      Utils::PrintException (e);
      return (1);
    }
  catch (const exception & e)
    {
      Utils::PrintException (e);
      return (1);
    }
  catch (int exitCode)
    {
      return (exitCode);
    }
}
