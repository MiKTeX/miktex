/**
 * @file texmfapp.cpp
 * @author Christian Schenk
 * @brief TeX'n'Friends helpers
 *
 * @copyright Copyright © 1996-2022 Christian Schenk
 *
 * This file is part of the MiKTeX TeXMF Framework.
 *
 * The MiKTeX TeXMF Framework is licensed under GNU General Public License
 * version 2 or any later version.
 */

#include <miktex/Core/Paths>
#include <miktex/Core/StreamReader>

#if defined(MIKTEX_TEXMF_SHARED)
#   define C4PEXPORT MIKTEXDLLEXPORT
#else
#   define C4PEXPORT
#endif
#define C1F0C63F01D5114A90DDF8FC10FF410B
#include "miktex/C4P/C4P.h"

#if defined(MIKTEX_TEXMF_SHARED)
#   define MIKTEXMFEXPORT MIKTEXDLLEXPORT
#else
#   define MIKTEXMFEXPORT
#endif
#define B8C7815676699B4EA2DE96F0BD727276
#include "miktex/TeXAndFriends/TeXMFApp.h"

#include "internal.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

typedef C4P_FILE_STRUCT(unsigned char) bytefile;
typedef C4P::C4P_text alphafile;

bool MIKTEXCEECALL MiKTeX::TeXAndFriends::OpenTFMFile(void* p, const PathName& fileName)
{
    MIKTEX_API_BEGIN("OpenTFMFile");
    MIKTEX_ASSERT_BUFFER(p, sizeof(bytefile));
    return TeXMFApp::GetTeXMFApp()->OpenFontFile(reinterpret_cast<bytefile*>(p), fileName.GetData(), FileType::TFM, MIKTEX_MAKETFM_EXE);
    MIKTEX_API_END("OpenTFMFile");
}

int MIKTEXCEECALL MiKTeX::TeXAndFriends::OpenXFMFile(void* ptr, const PathName& fileName)
{
    MIKTEX_API_BEGIN("OpenXFMFile");
    MIKTEX_ASSERT_BUFFER(ptr, sizeof(bytefile));
    if (TeXMFApp::GetTeXMFApp()->OpenFontFile(reinterpret_cast<bytefile*>(ptr), fileName.GetData(), FileType::TFM, MIKTEX_MAKETFM_EXE))
    {
        return 1;
    }
    if (TeXMFApp::GetTeXMFApp()->OpenFontFile(reinterpret_cast<bytefile*>(ptr), fileName.GetData(), FileType::OFM, MIKTEX_MAKETFM_EXE))
    {
        return 2;
    }
    return 0;
    MIKTEX_API_END("OpenXFMFile");
}

bool MIKTEXCEECALL MiKTeX::TeXAndFriends::OpenVFFile(void* ptr, const PathName& fileName)
{
    MIKTEX_API_BEGIN("OpenVFFile");
    MIKTEX_ASSERT_BUFFER(ptr, sizeof(bytefile));
    return TeXMFApp::GetTeXMFApp()->OpenFontFile(reinterpret_cast<bytefile*>(ptr), fileName.GetData(), FileType::VF, nullptr);
    MIKTEX_API_END("OpenVFFile");
}

int MIKTEXCEECALL MiKTeX::TeXAndFriends::OpenXVFFile(void* ptr, const PathName& fileName)
{
    MIKTEX_API_BEGIN("OpenXVFFile");
    MIKTEX_ASSERT_BUFFER(ptr, sizeof(bytefile));
    return TeXMFApp::GetTeXMFApp()->OpenFontFile(reinterpret_cast<bytefile*>(ptr), fileName.GetData(), FileType::VF, nullptr)
        || TeXMFApp::GetTeXMFApp()->OpenFontFile(reinterpret_cast<bytefile*>(ptr), fileName.GetData(), FileType::OVF, nullptr);
    MIKTEX_API_END("OpenXVFFile");
}

STATICFUNC(bool) OpenAlphaFile(void* p, const char* lpszFileName, FileType fileType, const char* lpszExtension)
{
    MIKTEX_ASSERT(p != nullptr);
    MIKTEX_ASSERT_BUFFER(p, sizeof(alphafile));
    MIKTEX_ASSERT_STRING(lpszFileName);
    shared_ptr<Session> session = MIKTEX_SESSION();
    PathName fileName(lpszFileName);
    if (!fileName.HasExtension() && lpszExtension != nullptr)
    {
        fileName.SetExtension(lpszExtension);
    }
    PathName path;
    if (!session->FindFile(fileName.ToString(), fileType, path))
    {
        return false;
    }
    FILE* pfile = session->TryOpenFile(path, FileMode::Open, FileAccess::Read, false);
    if (pfile == nullptr)
    {
        return false;
    }
    reinterpret_cast<alphafile*>(p)->Attach(pfile, true);
    reinterpret_cast<alphafile*>(p)->Read();
    return true;
}

#if 0
bool MIKTEXCEECALL MiKTeX::TeXAndFriends::OpenMAPFile(void* p, const char* lpszFileName)
{
    MIKTEX_API_BEGIN("OpenMAPFile");
    return OpenAlphaFile(p, lpszFileName, FileType::MAP, ".map");
    MIKTEX_API_END("OpenMAPFile");
}

bool MIKTEXCEECALL MiKTeX::TeXAndFriends::OpenMETAFONTFile(void* p, const char* lpszFileName)
{
    MIKTEX_API_BEGIN("OpenMETAFONTFile");
    return OpenAlphaFile(p, lpszFileName, FileType::MF, ".mf");
    MIKTEX_API_END("OpenMETAFONTFile");
}
#endif