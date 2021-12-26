/* internal.h:

   Copyright (C) 2021 Christian Schenk

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

#pragma once

#include <memory>
#include <string>

#include <miktex/Definitions>

#include <miktex/Core/Quoter>
#include <miktex/Core/Session>

#define Q_(x) MiKTeX::Core::Quoter<char>(x).GetData()
#define T_(x) MIKTEXTEXT(x)

namespace OneMiKTeXUtility
{
    class MIKTEXNOVTABLE Program
    {
    public:
        virtual bool Canceled() = 0;
        virtual std::string InvocationName() = 0;
    };

    class MIKTEXNOVTABLE Installer
    {
    public:
        virtual void EnableInstaller(bool b) = 0;
    };

    class MIKTEXNOVTABLE Logger
    {
    public:
        virtual void LogFatal(const std::string& message) = 0;
        virtual void LogInfo(const std::string& message) = 0;
    };

    class MIKTEXNOVTABLE UI
    {
    public:
        virtual MIKTEXNORETURN void BadUsage(const std::string& message, const std::string& usageSyntax) = 0;
        virtual MIKTEXNORETURN void FatalError(const std::string& message) = 0;
        virtual void Output(const std::string& s) = 0;
        virtual void Verbose(int level, const std::string& message) = 0;
        virtual int VerbosityLevel() = 0;
        virtual void Warning(const std::string& message) = 0;
    };

    struct ApplicationContext
    {
        Installer* installer;
        Logger* logger;
        Program* program;
        std::shared_ptr<MiKTeX::Core::Session> session;
        UI* ui;
    };
}
