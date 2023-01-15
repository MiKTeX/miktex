/**
 * @file Setup.h
 * @author Christian Schenk
 * @brief Pre-compiled header stuff
 *
 * @copyright Copyright © 1999-2023 Christian Schenk
 *
 * This file is part of MiKTeX Setup Wizard.
 *
 * MiKTeX Setup Wizard is licensed under GNU General Public License version 2 or
 * any later version.
 */

#pragma once

#if !defined(_SECURE_ATL)
#define _SECURE_ATL 1
#endif

#if !defined(VC_EXTRALEAN)
#define VC_EXTRALEAN
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS

#define _AFX_ALL_WARNINGS

#include <afxwin.h>
#include <afxext.h>
#include <afxinet.h>
#include <afxdtctl.h>
#include <afxmt.h>
#include <afxtempl.h>

#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>
#endif

#include <intshcut.h>
#define SECURITY_WIN32
#include <Security.h>

#include <regstr.h>
#include <shlobj.h>

#include <comdef.h>
#include <comip.h>

#include <algorithm>
#include <memory>
#include <vector>

#include <getopt.h>

#include "Setup-version.h"

#include <miktex/Core/AutoResource>
#include <miktex/Core/BufferSizes>
#include <miktex/Core/Cfg>
#include <miktex/Core/CommandLineBuilder>
#include <miktex/Core/Debug>
#include <miktex/Core/Directory>
#include <miktex/Core/DirectoryLister>
#include <miktex/Core/Exceptions>
#include <miktex/Core/File>
#include <miktex/Core/Paths>
#include <miktex/Core/Process>
#include <miktex/Core/Quoter>
#include <miktex/Core/StreamReader>
#include <miktex/Core/TemporaryDirectory>
#include <miktex/Core/Urls>
#include <miktex/Core/Utils>
#include <miktex/Core/win/HResult>
#include <miktex/Core/win/WindowsVersion>
#include <miktex/PackageManager/PackageManager>
#include <miktex/Setup/SetupService>
#include <miktex/Trace/TraceStream>
#include <miktex/Util/StringUtil>

using namespace MiKTeX::Configuration;
using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace MiKTeX::Setup;
using namespace MiKTeX::Trace;
using namespace MiKTeX::Util;
using namespace std;
