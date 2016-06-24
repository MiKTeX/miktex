/* winCabExtractor.cpp: using the Fdi lib to extract cab files

   Copyright (C) 2001-2016 Christian Schenk

   This file is part of MiKTeX Extractor.

   MiKTeX Extractor is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.
   
   MiKTeX Extractor is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with MiKTeX Extractor; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#include "StdAfx.h"

#if defined(ENABLE_WINDOWS_CAB_EXTRACTOR)

#include "internal.h"

#include "win/winCabExtractor.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Extractor;
using namespace std;

#define ERROR_TITLE T_("Cabinet Extraction Problem")
#define CRLF "\r\n"

BEGIN_INTERNAL_NAMESPACE;

/* _________________________________________________________________________

   openFiles
   _________________________________________________________________________ */

map<int, PathName> winCabExtractor::openFiles;

/* _________________________________________________________________________

   winCabExtractor::winCabExtractor
   _________________________________________________________________________ */

winCabExtractor::winCabExtractor ()
  : error (false),
    cancelled (false),
    makeDirectories (true),
    uncompressedSize (0),
    hfdi (0),
    fileCount (0),
    pCallback (0),
    trace_error (TraceStream::Open("error")),
    trace_extractor (TraceStream::Open("extractor"))
{
  memset (&erf, 0, sizeof(erf));
}

/* _________________________________________________________________________

   winCabExtractor::~winCabExtractor
   _________________________________________________________________________ */

winCabExtractor::~winCabExtractor ()
{
  try
    {
      Dispose ();
    }
  catch (const exception &)
    {
    }
}

/* _________________________________________________________________________

   winCabExtractor::Dispose
   _________________________________________________________________________ */

void
winCabExtractor::Dispose ()
{
  if (hfdi != 0)
    {
      HFDI hfdi = this->hfdi;
      this->hfdi = 0;
      if (! FDIDestroy(hfdi))
	{
	  FATAL_EXTRACTOR_ERROR
	    ("winCabExtractor::Dispose",
	     T_("The cabinet extractor could not be destroyed."),
	     0);
	}
    }
  if (trace_error.get() != 0)
    {
      trace_error->Close ();
      trace_error.reset ();
    }
  if (trace_extractor.get() != 0)
    {
      trace_extractor->Close ();
      trace_extractor.reset ();
    }
}

/* _________________________________________________________________________

   winCabExtractor::FatalFdiError
   _________________________________________________________________________ */

void
MIKTEXNORETURN
winCabExtractor::FatalFdiError (/*[in]*/ const char * lpszCabinetPath)
{
  if (! erf.fError)
    {
      FATAL_EXTRACTOR_ERROR ("winCabExtractor::FatalFdiError",
			     T_("Cabinet extraction problem."),
			     lpszCabinetPath);
    }
  string message;
  switch (erf.erfOper)
    {
    case FDIERROR_CABINET_NOT_FOUND:
      message = T_("Cabinet not found.");
      break;
    case FDIERROR_NOT_A_CABINET:
      message = T_("Cabinet file does not have the correct format.");
      break;
    case FDIERROR_UNKNOWN_CABINET_VERSION:
      message = T_("Cabinet file has an unknown version number.");
      break;
    case FDIERROR_CORRUPT_CABINET:
      message = T_("Cabinet file is corrupt.");
      break;
    case FDIERROR_ALLOC_FAIL:
      message = T_("Could not allocate enough memory.");
      break;
    case FDIERROR_BAD_COMPR_TYPE:
      message = T_("Unknown compression type in a cabinet folder.");
      break;
    case FDIERROR_MDI_FAIL:
      message = T_("Failure decompressing data from a cabinet file.");
      break;
    case FDIERROR_TARGET_FILE:
      message = T_("Failure writing to target file.");
      break;
    case FDIERROR_RESERVE_MISMATCH:
      message = T_("Cabinets in a set do not have the same RESERVE sizes.");
      break;
    case FDIERROR_WRONG_CABINET:
      message = T_("Cabinet returned on fdintNEXT_CABINET is incorrect.");
      break;
    case FDIERROR_USER_ABORT:
      message = T_("FDI aborted.");
      break;
    default:
    case FDIERROR_NONE:
      message = T_("Cabinet extraction problem.");
      break;
    }
  FATAL_EXTRACTOR_ERROR ("winCabExtractor::FatalFdiError",
			 message.c_str(),
			 lpszCabinetPath);
}

/* _________________________________________________________________________

   winCabExtractor::Extract
   _________________________________________________________________________ */

void
winCabExtractor::Extract (/*[in]*/ const PathName &	cabinetPath,
			  /*[in]*/ const PathName &	destDir,
			  /*[in]*/ bool			makeDirectories,
			  /*[in]*/ IExtractCallback *	pCallback,
			  /*[in]*/ const char *	lpszPrefix)
{
  trace_extractor->WriteFormattedLine
    ("libextractor",
     T_("extracting %s to %s (%s)"),
     Q_(cabinetPath),
     Q_(destDir),
     (makeDirectories
      ? T_("make directories")
      : T_("don't make directories")));
  hfdi = FDICreate(Alloc, Free, Open, Read, Write, Close, Seek, 0, &erf);
  if (hfdi == 0)
    {
      FATAL_EXTRACTOR_ERROR ("winCabExtractor::Extract",
			     T_("The cabinet extractor could not be created."),
			     0);
    }
  destinationDirectory = destDir;
  this->makeDirectories = makeDirectories;
  this->pCallback = pCallback;
  if (lpszPrefix != 0)
    {
      prefixToBeStripped = lpszPrefix;
    }
  PathName pathCabName (cabinetPath);
  pathCabName.RemoveDirectorySpec ();
  PathName path (cabinetPath);
  path.RemoveFileSpec ();
  path.AppendDirectoryDelimiter ();
  fileCount = 0;
  BOOL ok =
    FDICopy(hfdi,
	    const_cast<char*>(pathCabName.Get()),
	    const_cast<char*>(path.Get()),
	    0,
	    Notify,
	    0,
	    this);
  if (error)
    {
      FATAL_EXTRACTOR_ERROR ("winCabExtractor::Extract",
			     T_("Cabinet extraction problem."),
			     pathCabName.Get());
    }
  if (cancelled)
    {
      throw OperationCancelledException ();
    }
  if (! ok)
    {
      FatalFdiError (cabinetPath.Get());
    }
  trace_extractor->WriteFormattedLine ("libextractor",
				       T_("%u file(s)"),
				       static_cast<unsigned>(fileCount));
}

/* _________________________________________________________________________

   winCabExtractor::Extract
   _________________________________________________________________________ */

void
winCabExtractor::Extract (/*[in]*/ Stream *		pStream,
			  /*[in]*/ const PathName &	destDir,
			  /*[in]*/ bool			makeDirectories,
			  /*[in]*/ IExtractCallback *	pCallback,
			  /*[in]*/ const char *		lpszPrefix)
{
  UNUSED_ALWAYS (pStream);
  UNUSED_ALWAYS (destDir);
  UNUSED_ALWAYS (makeDirectories);
  UNUSED_ALWAYS (pCallback);
  UNUSED_ALWAYS (lpszPrefix);
  UNIMPLEMENTED ("winCabExtractor::Extract");
}

/* _________________________________________________________________________

   winCabExtractor::Alloc
   _________________________________________________________________________ */

FNALLOC(winCabExtractor::Alloc)
{
  try
    {
      void * pv = malloc(cb);
      if (pv == 0)
	{
	  OUT_OF_MEMORY ("winCabExtractor::Alloc");
	}
      return (pv);
    }
  catch (const exception &)
    {
      return (0);
    }
}

/* _________________________________________________________________________

   winCabExtractor::Free
   _________________________________________________________________________ */

FNFREE(winCabExtractor::Free)
{
  try
    {
      free (pv);
    }
  catch (const exception &)
    {
    }
}

/* _________________________________________________________________________

   winCabExtractor::Open
   _________________________________________________________________________ */

FNOPEN(winCabExtractor::Open)
{
  try
    {
      MIKTEX_ASSERT_STRING (pszFile);
      int fd = -1;
      do
	{
#if _MSC_VER >= 1400
	  if (_sopen_s(&fd,
		       pszFile,
		       oflag,
		       _SH_DENYWR,
		       pmode)
	      != 0)
	    {
	      fd = -1;
	    }
#else
	  fd = _open(pszFile, oflag, pmode);
#endif
	  if (fd >= 0)
	    {
	      openFiles[fd] = pszFile;
	    }
	  else
	    {
	      int err = errno;
	      CRT_ERROR ("open", pszFile);
	      string errorMessage;
	      ostringstream text;
	      text
		<< T_("\
The operation could not be completed because the following file could not \
be accessed:")
		<< CRLF
		<< CRLF
		<< pszFile
		<< CRLF
		<< CRLF
		<< T_("\
Make sure that no other application uses the file and that you have \
permission to access the file.");
	      if (MessageBoxA(0,
			      text.str().c_str(),
			      ERROR_TITLE,
			      MB_ICONSTOP | MB_RETRYCANCEL)
		  != IDRETRY)
		{
		  break;
		}
	    }
	}
      while (fd < 0);
      return (fd);
    }
  catch (const exception &)
    {
      return (-1);
    }
}

/* _________________________________________________________________________

   winCabExtractor::Read
   _________________________________________________________________________ */

FNREAD(winCabExtractor::Read)
{
  MIKTEX_ASSERT_BUFFER (pv, cb);
  try
    {
      int ret = -1;
      do
	{
	  ret = _read(static_cast<int>(hf), pv, cb);
	  if (ret < 0)
	    {
	      int err = errno;
	      const char * lpszFileName = openFiles[hf].Get();
	      CRT_ERROR ("read", lpszFileName);
	      string errorMessage;
	      ostringstream text;
	      text
		<< T_("\
The operation could not be completed because the following file could not \
be read:")
		<< CRLF
		<< CRLF
		<< lpszFileName
		<< CRLF
		<< CRLF
		<< T_("Reason: ")
		<< GetErrnoMessage(err, errorMessage);
	      if (MessageBoxA(0,
			      text.str().c_str(),
			      ERROR_TITLE,
			      MB_ICONSTOP | MB_RETRYCANCEL)
		  != IDRETRY)
		{
		  break;
		}
	    }
	}
      while (ret < 0);
      return (ret);
    }
  catch (const exception &)
    {
      return (static_cast<unsigned int>(-1));
    }
}

/* _________________________________________________________________________

   winCabExtractor::Write
   _________________________________________________________________________ */

FNWRITE(winCabExtractor::Write)
{
  MIKTEX_ASSERT_BUFFER (pv, cb);
  try
    {
      int ret = -1;
      do
	{
	  ret = _write(static_cast<int>(hf), pv, cb);
	  if (static_cast<unsigned>(ret) != cb)
	    {
	      int err = errno;
	      const char * lpszFileName = openFiles[hf].Get();
	      CRT_ERROR ("write", lpszFileName);
	      string errorMessage;
	      ostringstream text;
	      text
		<< T_("\
The operation could not be completed because the following file could not \
be written:")
		<< CRLF
		<< CRLF
		<< lpszFileName
		<< CRLF
		<< CRLF
		<< T_("Reason: ")
		<< GetErrnoMessage(err, errorMessage);
	      if (MessageBoxA(0,
			      text.str().c_str(),
			      ERROR_TITLE,
			      MB_ICONSTOP | MB_RETRYCANCEL)
		  != IDRETRY)
		{
		  break;
		}
	    }
	}
      while (ret < 0);
      return (ret);
    }
  catch (const exception &)
    {
      return (static_cast<unsigned int>(-1));
    }
}

/* _________________________________________________________________________

   winCabExtractor::Close
   _________________________________________________________________________ */

FNCLOSE(winCabExtractor::Close)
{
  try
    {
      int ret = _close(static_cast<int>(hf));
      if (ret != 0)
	{
	  CRT_ERROR ("close", openFiles[hf].Get());
	}
      openFiles.erase (hf);
      return (ret);
    }
  catch (const exception &)
    {
      return (-1);
    }
}

/* _________________________________________________________________________

   winCabExtractor::Seek
   _________________________________________________________________________ */

FNSEEK(winCabExtractor::Seek)
{
  try
    {
      int ret = -1;
      do
	{
	  ret = _lseek(static_cast<int>(hf), dist, seektype);
	  if (ret < 0)
	    {
	      int err = errno;
	      const char * lpszFileName = openFiles[hf].Get();
	      CRT_ERROR ("seek", lpszFileName);
	      string errorMessage;
	      ostringstream text;
	      text
		<< T_("\
The operation could not be completed because the following file could not \
be read:")
		<< CRLF
		<< CRLF
		<< lpszFileName
		<< CRLF
		<< CRLF
		<< T_("Reason: ")
		<< GetErrnoMessage(err, errorMessage);
	      if (MessageBoxA(0,
			      text.str().c_str(),
			      ERROR_TITLE,
			      MB_ICONSTOP | MB_RETRYCANCEL)
		  != IDRETRY)
		{
		  break;
		}
	    }
	}
      while (ret < 0);
      return (ret);
    }
  catch (const exception &)
    {
      return (-1);
    }
}

/* _________________________________________________________________________

   winCabExtractor::Notify
   _________________________________________________________________________ */

#define GET_OSFHANDLE(hf) \
  reinterpret_cast<HANDLE>(_get_osfhandle(static_cast<int>(hf)))

FNFDINOTIFY(winCabExtractor::Notify)
{
  winCabExtractor * This = reinterpret_cast<winCabExtractor*>(pfdin->pv);

  try
    {
      switch (fdint)
	{

	case fdintCOPY_FILE:

	  {
	    if (This->error || This->cancelled)
	      {
		return (-1);
	      }

	    MIKTEX_ASSERT_STRING (pfdin->psz1);

	    const char * lpszPath = pfdin->psz1;

	    // skip prefix directory
	    if (PathName::Compare(This->prefixToBeStripped.c_str(),
				  pfdin->psz1,
				  static_cast<unsigned long>
				  (This->prefixToBeStripped.length()))
		== 0)
	      {
		lpszPath += This->prefixToBeStripped.length();
	      }

	    // make the destination path name
	    PathName path (This->destinationDirectory);
	    if (! This->makeDirectories)
	      {
		path += PathName(lpszPath).RemoveDirectorySpec().Get();
	      }
	    else
	      {
		path += lpszPath;
	      }
	
	    This->uncompressedSize = pfdin->cb;

	    // notify client
	    if (This->pCallback != 0)
	      {
		This->pCallback->OnBeginFileExtraction
		  (path.Get(),
		   This->uncompressedSize);
	      }

	    // create the destination directory
	    PathName pathDestDir (path);
	    pathDestDir.RemoveFileSpec ();
	    bool haveDirectory = false;
	    do
	      {
		try
		  {
		    Directory::Create (pathDestDir);
		    haveDirectory = true;
		  }
		catch (exception & e)
		  {
		    if (This->pCallback == 0)
		      {
			throw;
		      }
		    ostringstream text;
		    text
		      << T_("\
The operation could not be completed because the following directory \
could not be created:")
		      << CRLF
		      << CRLF
		      << pathDestDir.Get()
		      << CRLF
		      << CRLF
		      << T_("Reason: ")
		      << e.what()
		      << CRLF
		      << CRLF
		      << T_("\
Make sure you have permission to create this directory.");
		    if (! This->pCallback->OnError(text.str().c_str()))
		      {
			throw;
		      }
		  }
	      }
	    while (! haveDirectory);

	    // reset the read-only attribute
	    if (File::Exists(path))
	      {
		FileAttributes attributes = File::GetAttributes(path);
		if ((attributes & FileAttributes::ReadOnly) != 0
		    || (attributes & FileAttributes::Hidden) != 0)
		  {
		    attributes &= ~ FileAttributes(FileAttributes::ReadOnly);
		    attributes &= ~ FileAttributes(FileAttributes::Hidden);
		    File::SetAttributes (path, attributes);
		  }
	      }

	    // open the destination file
	    return (Open(path.GetBuffer(),
			 (_O_BINARY | _O_CREAT | _O_TRUNC | _O_WRONLY),
			 _S_IREAD | _S_IWRITE));
	  }
      
	case fdintCLOSE_FILE_INFO:

	  {
	    // fast exit on error
	    if (This->error || This->cancelled)
	      {
		Close (pfdin->hf);
		return (-1);
	      }

	    // set the file time
	    if (_commit(static_cast<int>(pfdin->hf)) != 0)
	      {
		CRT_ERROR ("_commit", 0);
	      }
	    FILETIME filetime;
	    if (! DosDateTimeToFileTime(pfdin->date, pfdin->time, &filetime))
	      {
		WINDOWS_ERROR ("DosDateTimeToFileTime", 0);
	      }
	    else
	      {
		FILETIME filetimeLocal;
		if (! LocalFileTimeToFileTime(&filetime, &filetimeLocal))
		  {
		    WINDOWS_ERROR ("LocalFileTimeToFileTime", 0);
		  }
		else
		  {
		    if (! SetFileTime(GET_OSFHANDLE(pfdin->hf),
				      &filetimeLocal,
				      0,
				      &filetimeLocal))
		      {
			WINDOWS_ERROR ("SetFileTime", 0);
		      }
		  }
	      }
	
	    // close the destination file
	    Close (pfdin->hf);

	    // skip the prefix directory
	    MIKTEX_ASSERT_STRING (pfdin->psz1);
	    const char * lpszPath = pfdin->psz1;
	    if (PathName::Compare(This->prefixToBeStripped.c_str(),
				  pfdin->psz1,
				  static_cast<unsigned long>
				  (This->prefixToBeStripped.length()))
		== 0)
	      {
		lpszPath += This->prefixToBeStripped.length();
	      }

	    // make the path name of the destination file
	    PathName path (This->destinationDirectory);
	    if (! This->makeDirectories)
	      {
		path += PathName(lpszPath).RemoveDirectorySpec().Get();
	      }
	    else
	      {
		path += lpszPath;
	      }

	    // set file attributes
	    unsigned long attributes = 0;
	    if ((pfdin->attribs & _A_RDONLY) != 0)
	      {
		attributes |= FILE_ATTRIBUTE_READONLY;
	      }
	    if ((pfdin->attribs & _A_SYSTEM) != 0)
	      {
		attributes |= FILE_ATTRIBUTE_SYSTEM;
	      }
	    if ((pfdin->attribs & _A_HIDDEN) != 0)
	      {
		attributes |= FILE_ATTRIBUTE_HIDDEN;
	      }
	    if ((pfdin->attribs & _A_ARCH) != 0)
	      {
		attributes |= FILE_ATTRIBUTE_ARCHIVE;
	      }
	    if (! SetFileAttributes(path.Get(), attributes))
	      {
		WINDOWS_ERROR ("SetFileAttributes", path.Get());
	      }

	    // notify the client
	    if (This->pCallback != 0)
	      {
		This->pCallback->OnEndFileExtraction (0,
						      This->uncompressedSize);
	      }

	    This->fileCount += 1;

	    return (TRUE);
	  }
	
	default:

	  return (0);

	}
    }
  catch (const OperationCancelledException &)
    {
      This->trace_extractor->WriteFormattedLine ("libextractor",
			      T_("operation cancelled by client"));
      This->cancelled = true;
      return (-1);
    }
  catch (const exception &)
    {
      This->error = true;
      return (-1);
    }
}

END_INTERNAL_NAMESPACE;

#endif
