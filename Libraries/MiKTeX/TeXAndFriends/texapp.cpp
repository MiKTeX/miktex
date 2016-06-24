/* texapp.cpp:

   Copyright (C) 1996-2016 Christian Schenk

   This file is part of the MiKTeX TeXMF Library.

   The MiKTeX TeXMF Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX TeXMF Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX TeXMF Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#include "StdAfx.h"

#include "internal.h"

#define EXPERT_SRC_SPECIALS 0

TeXApp::TeXApp()
  : write18Mode(Write18Mode::Disabled)
{
}

void TeXApp::Init(const char * lpszProgramInvocationName)
{
  TeXMFApp::Init(lpszProgramInvocationName);

  SetTeX();

  enableEncTeX = false;
  enableMLTeX = false;
  write18Mode = Write18Mode::Disabled;
  lastLineNum = -1;
  param_font_max = -1;
  param_font_mem_size = -1;
  param_hyph_size = -1;
  param_max_in_open = -1;
  param_mem_bot = -1;
  param_nest_size = -1;
  param_save_size = -1;
  param_trie_op_size = -1;
  param_trie_size = -1;
  param_hash_extra = -1;
# define SYNCTEX_NO_OPTION INT_MAX
  synchronizationOptions = SYNCTEX_NO_OPTION;

  if (AmI("omega"))
  {
    IsNameManglingEnabled = true;
  }
}

void TeXApp::OnTeXMFStartJob()
{
  TeXMFApp::OnTeXMFStartJob();
  string enableWrite18 = session->GetConfigValue(nullptr, MIKTEX_REGVAL_ENABLE_WRITE18, texapp::texapp::EnableWrite18().c_str());
  if (enableWrite18 == "t")
  {
    write18Mode = Write18Mode::Enabled;
  }
  else if (enableWrite18 == "f")
  {
    write18Mode = Write18Mode::Disabled;
  }
  else if (enableWrite18 == "p")
  {
    write18Mode = Write18Mode::PartiallyEnabled;
  }
  else if (enableWrite18 == "q")
  {
    write18Mode = Write18Mode::Query;
  }
  else
  {
    MIKTEX_UNEXPECTED();
  }
}

void TeXApp::Finalize()
{
  lastSourceFilename = "";
  sourceSpecials.reset();
  TeXMFApp::Finalize();
}

enum {
  OPT_DISABLE_WRITE18,
  OPT_ENABLE_ENCTEX,
  OPT_ENABLE_MLTEX,
  OPT_ENABLE_WRITE18,
  OPT_FONT_MAX,
  OPT_FONT_MEM_SIZE,
  OPT_HASH_EXTRA,
  OPT_MAX_IN_OPEN,
  OPT_MEM_BOT,
  OPT_NEST_SIZE,
  OPT_RESTRICT_WRITE18,
  OPT_SAVE_SIZE,
  OPT_SRC_SPECIALS,
  OPT_SYNCTEX,
  OPT_TRIE_OP_SIZE,
  OPT_TRIE_SIZE,
};

void TeXApp::AddOptions()
{
  TeXMFApp::AddOptions();

  optBase = static_cast<int>(GetOptions().size());

  AddOption(T_("disable-write18\0Disable the \\write18{COMMAND} construct."),
    FIRST_OPTION_VAL + optBase + OPT_DISABLE_WRITE18);

  AddOption(T_("enable-enctex\0Enable EncTeX extensions such as \\mubyte."),
    FIRST_OPTION_VAL + optBase + OPT_ENABLE_ENCTEX);

  AddOption(T_("enable-mltex\0Enable MLTeX extensions such as \\charsubdef."),
    FIRST_OPTION_VAL + optBase + OPT_ENABLE_MLTEX);

  AddOption(T_("enable-write18\0Enable the \\write18{COMMAND} construct."),
    FIRST_OPTION_VAL + optBase + OPT_ENABLE_WRITE18);

  AddOption(T_("hash-extra\0Set hash_extra to N."),
    FIRST_OPTION_VAL + optBase + OPT_HASH_EXTRA,
    POPT_ARG_STRING,
    "N");

  AddOption(T_("max-in-open\0Set max_in_open to N."),
    FIRST_OPTION_VAL + optBase + OPT_MAX_IN_OPEN,
    POPT_ARG_STRING,
    "N");

  AddOption(T_("mem-bot\0Set mem_bot to 0 or 1."),
    FIRST_OPTION_VAL + optBase + OPT_MEM_BOT,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN,
    "N");

  AddOption(T_("nest-size\0Set nest_size to N."),
    FIRST_OPTION_VAL + optBase + OPT_NEST_SIZE,
    POPT_ARG_STRING,
    "N");

  AddOption(T_("restrict-write18\0Partially enable the \\write18{COMMAND} construct."),
    FIRST_OPTION_VAL + optBase + OPT_RESTRICT_WRITE18);

  AddOption(T_("save-size\0Set save_size to N."),
    FIRST_OPTION_VAL + optBase + OPT_SAVE_SIZE,
    POPT_ARG_STRING,
    "N");

#if WITH_SYNCTEX
  if (AmI("xetex") || AmI("pdftex"))
  {
    AddOption(T_("synctex\0Generate SyncTeX data for previewers if nonzero."),
      FIRST_OPTION_VAL + optBase + OPT_SYNCTEX,
      POPT_ARG_STRING,
      "N");

  }
#endif

  AddOption(T_("trie-size\0Set trie_size to N."),
    FIRST_OPTION_VAL + optBase + OPT_TRIE_SIZE,
    POPT_ARG_STRING,
    "N");

  if (!AmI("omega"))
  {
    AddOption(T_("font-max\0Set font_max to N."),
      FIRST_OPTION_VAL + optBase + OPT_FONT_MAX,
      POPT_ARG_STRING,
      "N");
    AddOption(T_("font-mem-size\0Set font_mem_size to N."),
      FIRST_OPTION_VAL + optBase + OPT_FONT_MEM_SIZE,
      POPT_ARG_STRING,
      "N");
  }


  if (AmI("omega"))
  {
    AddOption(T_("trie-op-size\0Set trie_op_size to N."),
      FIRST_OPTION_VAL + optBase + OPT_TRIE_OP_SIZE,
      POPT_ARG_STRING,
      "N");
  }

#if EXPERT_SRC_SPECIALS
  AddOption((T_("src-specials\0Insert source specials in certain places of the DVI file.  WHERE is a comma-separated value list of: cr display hbox math par parend vbox.")),
    FIRST_OPTION_VAL + optBase + OPT_SRC_SPECIALS,
    POPT_ARG_STRING | POPT_ARGFLAG_OPTIONAL,
    "WHERE");
#else
  AddOption((T_("src-specials\0Insert source specials in certain places of the DVI file.")),
    FIRST_OPTION_VAL + optBase + OPT_SRC_SPECIALS);
#endif

  // obsolete options
  AddOption("try-gz\0", OPT_UNSUPPORTED);

  // old option names
  AddOption("src", "src-specials");

  // supported Web2C options
  AddOption("enc", "enable-enctex");
  AddOption("mltex", "enable-mltex");
  AddOption("fmt", "undump");
  AddOption("no-shell-escape", "disable-write18");
  AddOption("shell-restricted", "restrict-write18");
  AddOption("shell-escape", "enable-write18");

  // unsupported Web2C options
  AddOption("ipc", OPT_UNSUPPORTED);
  AddOption("ipc-start", OPT_UNSUPPORTED);
  AddOption("output-comment",
    OPT_UNSUPPORTED,
    POPT_ARG_STRING,
    "comment");
}

extern bool inParseFirstLine;

bool TeXApp::ProcessOption(int optchar, const string & optArg)
{
  bool done = true;
  switch (optchar - FIRST_OPTION_VAL - optBase)
  {
  case OPT_DISABLE_WRITE18:
    write18Mode = Write18Mode::Disabled;
    break;
  case OPT_ENABLE_WRITE18:
    if (!inParseFirstLine)
    {
      write18Mode = Write18Mode::Enabled;
    }
    break;
  case OPT_RESTRICT_WRITE18:
    write18Mode = Write18Mode::PartiallyEnabled;
    break;
  case OPT_FONT_MAX:
    param_font_max = std::stoi(optArg);
    break;
  case OPT_FONT_MEM_SIZE:
    param_font_mem_size = std::stoi(optArg);
    break;
  case OPT_HASH_EXTRA:
    param_hash_extra = std::stoi(optArg);
    break;
  case OPT_MAX_IN_OPEN:
    param_max_in_open = std::stoi(optArg);
    break;
  case OPT_MEM_BOT:
    param_mem_bot = std::stoi(optArg);
    break;
  case OPT_ENABLE_ENCTEX:
    enableEncTeX = true;
    break;
  case OPT_ENABLE_MLTEX:
    enableMLTeX = true;
    break;
  case OPT_NEST_SIZE:
    param_nest_size = std::stoi(optArg);
    break;
  case OPT_SAVE_SIZE:
    param_save_size = std::stoi(optArg);
    break;
  case OPT_SRC_SPECIALS:
#if EXPERT_SRC_SPECIALS
    sourceSpecials[SourceSpecial::Auto] = true;
    sourceSpecials[SourceSpecial::Paragraph] = true;
#else
    if (optArg.empty())
    {
      sourceSpecials[(size_t)SourceSpecial::Auto] = true;
      sourceSpecials[(size_t)SourceSpecial::Paragraph] = true;
    }
    else
    {
      Tokenizer tok(optArg.c_str(), ", ");
      while (tok.GetCurrent() != nullptr)
      {
	if (Utils::Equals(tok.GetCurrent(), "everypar")
	  || Utils::Equals(tok.GetCurrent(), "par"))
	{
	  sourceSpecials[(size_t)SourceSpecial::Auto] = true;
	  sourceSpecials[(size_t)SourceSpecial::Paragraph] = true;
	}
	else if (Utils::Equals(tok.GetCurrent(), "everyparend")
	  || Utils::Equals(tok.GetCurrent(), "parend"))
	{
	  sourceSpecials[(size_t)SourceSpecial::ParagraphEnd] = true;
	}
	else if (Utils::Equals(tok.GetCurrent(), "everycr")
	  || Utils::Equals(tok.GetCurrent(), "cr"))
	{
	  sourceSpecials[(size_t)SourceSpecial::CarriageReturn] = true;
	}
	else if (Utils::Equals(tok.GetCurrent(), "everymath")
	  || Utils::Equals(tok.GetCurrent(), "math"))
	{
	  sourceSpecials[(size_t)SourceSpecial::Math] = true;
	}
	else if (Utils::Equals(tok.GetCurrent(), "everyhbox")
	  || Utils::Equals(tok.GetCurrent(), "hbox"))
	{
	  sourceSpecials[(size_t)SourceSpecial::HorizontalBox] = true;
	}
	else if (Utils::Equals(tok.GetCurrent(), "everyvbox")
	  || Utils::Equals(tok.GetCurrent(), "vbox"))
	{
	  sourceSpecials[(size_t)SourceSpecial::VerticalBox] = true;
	}
	else if (Utils::Equals(tok.GetCurrent(), "everydisplay")
	  || Utils::Equals(tok.GetCurrent(), "display"))
	{
	  sourceSpecials[(size_t)SourceSpecial::Display] = true;
	}
	else
	{
	  MIKTEX_FATAL_ERROR_2(T_("Unknown source special."), "special", tok.GetCurrent());
	}
	++tok;
	}
      }
#endif // EXPERT_SRC_SPECIALS
    break;
  case OPT_SYNCTEX:
    synchronizationOptions = std::stoi(optArg);
    break;

  case OPT_TRIE_SIZE:
    param_trie_size = std::stoi(optArg);
    break;
  case OPT_TRIE_OP_SIZE:
    param_trie_op_size = std::stoi(optArg);
    break;
  default:
    done = TeXMFApp::ProcessOption(optchar, optArg);
    break;
    }
  return done;
  }

#if defined(MIKTEX_WINDOWS)
inline bool NeedsEscape(char ch)
{
  return
    ch == '&'
    || ch == '|'
    || ch == '%'
    || ch == '<'
    || ch == '>'
    || ch == ';'
    || ch == ','
    || ch == '('
    || ch == ')';
}
#endif

bool ParseCommand(const string & command, string & quotedCommand, string & executable)
{
#if defined(MIKTEX_WINDOWS)
  const char QUOTE = '"';
#else
  const char QUOTE = '\'';
#endif
  string::const_iterator it = command.begin();
  for (; it != command.end() && (*it == ' ' || *it == '\t'); ++it)
  {
  }
  quotedCommand = "";
  executable = "";
  for (; it != command.end() && *it != ' ' && *it != '\t'; ++it)
  {
    quotedCommand += *it;
    executable += *it;
  }
  for (; it != command.end() && (*it == ' ' || *it == '\t'); ++it)
  {
    quotedCommand += *it;
  }
  bool startOfArg = true;
  while (it != command.end())
  {
    if (*it == '\'')
    {
      // only " quatation is allowd
      return false;
    }
    if (*it == '"')
    {
      if (!startOfArg)
      {
	quotedCommand += QUOTE;
      }
      startOfArg = false;
      quotedCommand += QUOTE;
      ++it;
      while (it != command.end() && *it != '"')
      {
#if defined(MIKTEX_WINDOWS)
	if (NeedsEscape(*it))
	{
	  quotedCommand += '^';
	}
#endif
	quotedCommand += *it++;
      }
      if (it == command.end())
      {
	return false;
      }
      ++it;
      if (it != command.end() && !(*it == ' ' || *it == '\t'))
      {
	return false;
      }
  }
    else if (startOfArg && !(*it == ' ' || *it == '\t'))
    {
      startOfArg = false;
      quotedCommand += QUOTE;
#if defined(MIKTEX_WINDOWS)
      if (NeedsEscape(*it))
      {
	quotedCommand += '^';
      }
#endif
      quotedCommand += *it++;
    }
    else if (!startOfArg && (*it == ' ' || *it == '\t'))
    {
      startOfArg = true;
      quotedCommand += QUOTE;
      quotedCommand += *it++;
    }
    else
    {
#if defined(MIKTEX_WINDOWS)
      if (NeedsEscape(*it))
      {
	quotedCommand += '^';
      }
#endif
      quotedCommand += *it++;
    }
}
  if (!startOfArg)
  {
    quotedCommand += QUOTE;
  }
  return true;
}

TeXApp::Write18Result TeXApp::Write18(const char * lpszCommand, int & exitCode) const
{
  MIKTEX_ASSERT_STRING(lpszCommand);
  Write18Result result = Write18Result::Executed;
  string command = lpszCommand;
  switch (write18Mode)
  {
  case Write18Mode::Enabled:
    break;
  case Write18Mode::Disabled:
    MIKTEX_UNEXPECTED();
  case Write18Mode::Query:
  case Write18Mode::PartiallyEnabled:
  {
    string quotedCommand;
    string executable;
    if (!ParseCommand(command, quotedCommand, executable))
    {
      return Write18Result::QuotationError;
    }
    command = quotedCommand;
    if (write18Mode == Write18Mode::Query)
    {
      // todo
      return Write18Result::Disallowed;
    }
    else
    {
      bool allowed = StringUtil::Contains(session->GetConfigValue(nullptr, MIKTEX_REGVAL_ALLOWED_SHELL_COMMANDS, texapp::texapp::AllowedShellCommands().c_str()).c_str(), executable.c_str(), ",;:",
#if defined(MIKTEX_WINDOWS)
	true
#else
	false
#endif
	);
      if (!allowed)
      {
	return Write18Result::Disallowed;
      }
    }
    result = Write18Result::ExecutedAllowed;
    break;
  }
  default:
    MIKTEX_UNEXPECTED();
  }
  Process::ExecuteSystemCommand(command.c_str(), &exitCode);
  return result;
}

TeXApp::Write18Result TeXApp::Write18(const wchar_t * lpszCommand, int & exitCode) const
{
  MIKTEX_ASSERT_STRING(lpszCommand);
  CharBuffer<char> buf(lpszCommand);
  return Write18(buf.Get(), exitCode);
}
