/* unxFileSystemWatcher.cpp: file system watcher (Unx specials)

   Copyright (C) 2021 Christian Schenk

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

#include "internal.h"

#include "unxFileSystemWatcher.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

unique_ptr<FileSystemWatcher> FileSystemWatcher::Create()
{
  return make_unique<unxFileSystemWatcher>();
}

unxFileSystemWatcher::~unxFileSystemWatcher()
{
  try
  {
  }
  catch (const exception&)
  {
  }
}

void unxFileSystemWatcher::AddDirectory(const MiKTeX::Util::PathName& dir)
{
}

void unxFileSystemWatcher::Subscribe(MiKTeX::Core::FileSystemWatcherCallback* callback)
{
}

void unxFileSystemWatcher::Unsubscribe(MiKTeX::Core::FileSystemWatcherCallback* callback)
{
}

void unxFileSystemWatcher::Start()
{
}

void unxFileSystemWatcher::Stop()
{
}
