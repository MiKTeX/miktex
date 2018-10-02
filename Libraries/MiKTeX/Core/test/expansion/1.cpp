/* 1.cpp:

   Copyright (C) 1996-2018 Christian Schenk

   This file is part of the MiKTeX Core Library.

   The MiKTeX Core Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX Core Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX Core Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#include "config.h"

#include <miktex/Core/Test>

#include <cstdlib>

#include <future>
#include <memory>
#include <string>

#include <miktex/Core/HasNamedValues>
#include <miktex/Core/Session>

using namespace MiKTeX::Core;
using namespace MiKTeX::Test;
using namespace std;

class VarExpand : public MiKTeX::Core::HasNamedValues
{
public:
  VarExpand(shared_ptr<Session> session) :
    session(session)
  {
  }
private:
  shared_ptr<Session> session;
public:
  bool TryGetValue(const std::string& valueName, std::string& varValue) override
  {
    if (valueName == "jkl")
    {
      varValue = "mno";
      return true;
    }
    else if (valueName == "rst")
    {
      varValue = session->Expand("x${abc}x");
      return true;
    }
    else
    {
      return false;
    }
  }
public:
  std::string GetValue(const std::string& valueName) override
  {
    MIKTEX_UNEXPECTED();
  }
};

BEGIN_TEST_SCRIPT("expansion-1");

BEGIN_TEST_FUNCTION(1);
{
  TEST(pSession->Expand("$$") == "$");
  putenv("abc=def");
  TEST(pSession->Expand("$abc") == "def");
  TEST(pSession->Expand("$(abc)") == "def");
  TEST(pSession->Expand("${abc}") == "def");
  putenv("xyz=x$(abc)x");
  TEST(pSession->Expand("$xyz") == "xdefx");
  VarExpand expander(pSession);
  TEST(pSession->Expand("$xyz", &expander) == "xdefx");
  TEST(pSession->Expand("$jkl", &expander) == "mno");
  TEST(pSession->Expand("$rst", &expander) == "xdefx");
}
END_TEST_FUNCTION();

BEGIN_TEST_FUNCTION(2);
{
  std::future<std::string> fut1 = std::async([](std::shared_ptr<MiKTeX::Core::Session> pSession) { return pSession->Expand("ab{cd,ef}gh{ij,kl}mn", { ExpandOption::Braces }, nullptr); }, pSession);
  std::future<std::string> fut2 = std::async([](std::shared_ptr<MiKTeX::Core::Session> pSession) { return pSession->Expand("x{a,b{c,d}e}y", { ExpandOption::Braces }, nullptr); }, pSession);
  std::future<std::string> fut3 = std::async([](std::shared_ptr<MiKTeX::Core::Session> pSession) { return pSession->Expand("/texmf/tex/{plain,generic,}//", { ExpandOption::Braces }, nullptr); }, pSession);
  putenv("abc={d,e}");
  std::future<std::string> fut4 = std::async([](std::shared_ptr<MiKTeX::Core::Session> pSession) { return pSession->Expand("a{b,c}$abc", { ExpandOption::Braces, ExpandOption::Values }, nullptr); }, pSession);
  std::future<std::string> fut5 = std::async([](std::shared_ptr<MiKTeX::Core::Session> pSession) { return pSession->Expand("x{A,B{1,2}}y", { ExpandOption::Braces }, nullptr); }, pSession);
  std::future<std::string> fut6 = std::async([](std::shared_ptr<MiKTeX::Core::Session> pSession) { return pSession->Expand("x{A,B}{1,2}y", { ExpandOption::Braces }, nullptr); }, pSession);
  std::future<std::string> fut7 = std::async([](std::shared_ptr<MiKTeX::Core::Session> pSession) { return pSession->Expand("%R/texmf/tex//", { ExpandOption::Braces }, nullptr); }, pSession);
#if defined(MIKTEX_WINDOWS)
  TEST(fut1.get() == "abcdghijmn;abcdghklmn;abefghijmn;abefghklmn");
#else
  TEST(fut1.get() == "abcdghijmn:abcdghklmn:abefghijmn:abefghklmn");
#endif
#if defined(MIKTEX_WINDOWS)
  TEST(fut2.get() == "xay;xbcey;xbdey");
#else
  TEST(fut2.get() == "xay:xbcey:xbdey");
#endif
#if defined(MIKTEX_WINDOWS)
  TEST(fut3.get() == "/texmf/tex/plain//;/texmf/tex/generic//;/texmf/tex///");
#else
  TEST(fut3.get() == "/texmf/tex/plain//:/texmf/tex/generic//:/texmf/tex///");
#endif
#if defined(MIKTEX_WINDOWS)
  TEST(fut4.get() == "abd;abe;acd;ace");
#else
  TEST(fut4.get() == "abd:abe:acd:ace");
#endif
#if defined(MIKTEX_WINDOWS)
  TEST(fut5.get() == "xAy;xB1y;xB2y");
#else
  TEST(fut5.get() == "xAy:xB1y:xB2y");
#endif
#if defined(MIKTEX_WINDOWS)
  TEST(fut6.get() == "xA1y;xA2y;xB1y;xB2y");
#else
  TEST(fut6.get() == "xA1y:xA2y:xB1y:xB2y");
#endif
#if 0 // todo: kpathsea (right-to-left) expansion
  TEST(pSession->Expand("x{A,B}{1,2}y", ExpandOption::Braces, nullptr) == "xA1y;xB1y;xA2y;xB2y");
#endif
  TEST(Utils::IsAbsolutePath(fut7.get()));
}
END_TEST_FUNCTION();

BEGIN_TEST_PROGRAM();
{
  CALL_TEST_FUNCTION(1);
  CALL_TEST_FUNCTION(2);
}
END_TEST_PROGRAM();

END_TEST_SCRIPT();

RUN_TEST_SCRIPT();
