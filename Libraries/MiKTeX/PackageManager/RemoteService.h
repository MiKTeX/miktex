/* RemoteService.h:                                     -*- C++ -*-

   Copyright (C) 2016-2018 Christian Schenk

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

#if !defined(B24B3F8AAF90478B97B88D2695E620C2)
#define B24B3F8AAF90478B97B88D2695E620C2

BEGIN_INTERNAL_NAMESPACE;

class MIKTEXNOVTABLE RemoteService
{
public:
  virtual ~RemoteService() = 0;

public:
  virtual std::vector<MiKTeX::Packages::RepositoryInfo> GetRepositories(MiKTeX::Packages::RepositoryReleaseState repositoryReleaseState) = 0;

public:
  virtual std::string PickRepositoryUrl(MiKTeX::Packages::RepositoryReleaseState repositoryReleaseState) = 0;

public:
  virtual std::pair<bool, MiKTeX::Packages::RepositoryInfo> TryGetRepositoryInfo(const std::string& url) = 0;

public:
  virtual MiKTeX::Packages::RepositoryInfo Verify(const std::string& url) = 0;

public:
  static std::unique_ptr<RemoteService> Create(const std::string& endpoint, const MiKTeX::Packages::ProxySettings& proxySettings);
};

END_INTERNAL_NAMESPACE;

#endif
