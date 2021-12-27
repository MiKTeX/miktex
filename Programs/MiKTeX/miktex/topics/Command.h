/* topics/Command.h:

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

#include <string>
#include <vector>

#include <miktex/Definitions>

#include "internal.h"

namespace OneMiKTeXUtility::Topics
{
    class MIKTEXNOVTABLE Command
    {
    public:
        virtual MIKTEXTHISCALL ~Command() noexcept = 0;
        virtual std::string Description() = 0;
        virtual int MIKTEXTHISCALL Execute(OneMiKTeXUtility::ApplicationContext& ctx, const std::vector<std::string>& arguments) = 0;
        virtual std::string Name() = 0;
        virtual std::string Synopsis() = 0;
    };
}
