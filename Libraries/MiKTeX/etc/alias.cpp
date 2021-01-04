/* alias.cpp: function wrapper

   Copyright (C) 1999-2021 Christian Schenk

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

#include <memory>

#include <miktex/Core/Exceptions>
#include <miktex/Core/FileType>
#include <miktex/Core/Session>
#include <miktex/Definitions>
#include <miktex/Util/StringUtil>

#if defined(MIKTEX_WINDOWS)
#include <miktex/Core/win/COMInitializer>
#endif

#if !defined(FUNC)
#  define FUNC c4pmain
#endif

extern "C" int MIKTEXCEECALL FUNC(int argc, char* argv[]);

#if defined(_UNICODE)
#  define WRAPPER_MAIN wmain
#  define WRAPPER_CHAR wchar_t
#else
#  define WRAPPER_MAIN main
#  define WRAPPER_CHAR char
#endif

int MIKTEXCEECALL WRAPPER_MAIN(int argc, WRAPPER_CHAR* argv[])
{
#if defined(MIKTEX_WINDOWS)
  MiKTeX::Core::COMInitializer comInitializer();
  std::vector<std::string> utf8args;
  utf8args.reserve(argc);
#endif
  std::vector<char*> args;
  args.reserve(argc + 1);
  for (int idx = 0; idx < argc; ++idx)
  {
#if defined(MIKTEX_WINDOWS)
#if defined(_UNICODE)
    utf8args.push_back(MiKTeX::Util::StringUtil::WideCharToUTF8(argv[idx]));
#else
    utf8args.push_back(MiKTeX::Util::StringUtil::AnsiToUTF8(argv[idx]));
#endif
    args.push_back(const_cast<char*>(utf8args[idx].c_str()));
#else
    args.push_back(argv[idx]);
#endif
  }
  args.push_back(nullptr);
  return FUNC(argc, &args[0]);
}
