/* exceptions.h:                                        -*- C++ -*-

   Copyright (C) 2001-2018 Christian Schenk

   This file is part of MiKTeX Package Manager.

   MiKTeX Package Manager is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   MiKTeX Package Manager is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with MiKTeX Package Manager; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#if !defined(D32CB6D8FBC5418A94C5064391B8D7AC)
#define D32CB6D8FBC5418A94C5064391B8D7AC

#include <string>

#include <miktex/Core/Exceptions>

#include "text.h"

BEGIN_INTERNAL_NAMESPACE;

class NotFoundException :
  public MiKTeX::Core::MiKTeXException
{
public:
  NotFoundException() :
    MiKTeX::Core::MiKTeXException(T_("Requested resource not found."))
  {
  }

public:
  NotFoundException(const std::string& url) :
    MiKTeX::Core::MiKTeXException("", T_("Requested resource not found."), { "url", url }, MiKTeX::Core::SourceLocation())
  {
  }

public:
  NotFoundException(const std::string& programInvocationName, const std::string& message, const MiKTeX::Core::MiKTeXException::KVMAP& info, const MiKTeX::Core::SourceLocation& sourceLocation) :
    MiKTeX::Core::MiKTeXException(programInvocationName, message, info, sourceLocation)
  {
  }
};

END_INTERNAL_NAMESPACE;

#endif
