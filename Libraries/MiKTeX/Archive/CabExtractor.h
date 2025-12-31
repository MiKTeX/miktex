/**
 * @file CabExtractor.h
 * @defgroup MiKTeX Archive
 * @author Christian Schenk
 * @brief CabExtractor implementation for MiKTeX Archive
 *
 * @copyright Copyright © 2001-2025 Christian Schenk
 *
 * This file is part of the MiKTeX Archive Library.
 *
 * MiKTeX Archive Library is licensed under GNU General Public License version 2
 * or any later version.
 */


#pragma once

#include <mspack.h>

#include <miktex/Archive/Extractor>
#include <miktex/Trace/TraceStream>

BEGIN_INTERNAL_NAMESPACE;

class CabExtractor :
  public MiKTeX::Archive::Extractor
{

public:

    CabExtractor();
    MIKTEXTHISCALL ~CabExtractor() override;

    void MIKTEXTHISCALL Extract(const MiKTeX::Util::PathName& path, const MiKTeX::Util::PathName& destDir, bool makeDirectories, IExtractCallback* callback, const std::string& str) override;
    void MIKTEXTHISCALL Extract(MiKTeX::Core::Stream* stream, const MiKTeX::Util::PathName& destDir, bool makeDirectories, IExtractCallback* callback, const std::string& str) override;

private:

    mscab_decompressor* decompressor = nullptr;

    static struct mspack_file* Open(struct mspack_system* self, const char* fileName, int mode);
    static void Close(struct mspack_file* mspackFile);
    static int Read(struct mspack_file* mspackFile, void* data, int numBytes);
    static int Write(struct mspack_file* mspackFile, void* data, int numBytes);
    static int Seek(struct mspack_file* mspackFile, off_t offset, int mode);
    static off_t Tell(struct mspack_file* mspackFile);
    static void Message(struct mspack_file* mspackFile, const char* lpszFormat, ...);
    static void* Alloc(struct mspack_system* self, size_t numBytes);
    static void Free(void* pv);
    static void Copy(void* source, void* dest, size_t numBytes);

    struct MySystem : public mspack_system
    {
        CabExtractor* pCabExtractor = nullptr;
    };

    struct MyFile
    {
        std::string fileName;
        FILE* stdioFile = nullptr;
    };

    mspack_system mspackSystem;

    std::unique_ptr<MiKTeX::Trace::TraceStream> traceStream;

    std::unique_ptr<MiKTeX::Trace::TraceStream> traceStopWatch;
};

END_INTERNAL_NAMESPACE;
