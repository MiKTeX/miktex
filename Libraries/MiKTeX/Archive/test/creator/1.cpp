/**
 * @file test/creator/1.cpp
 * @author Christian Schenk
 * @brief Creator test
 *
 * @copyright Copyright © 2026 Christian Schenk
 *
 * This file is part of the MiKTeX Archive Library.
 *
 * MiKTeX Archive Library is licensed under GNU General Public License version 2
 * or any later version.
 */

#include "config.h"

#include <random>

#include <miktex/Core/Test>

#include <miktex/Archive/Creator>
#include <miktex/Core/File>
#include <miktex/Core/FileStream>

using namespace std;

using namespace MiKTeX::Archive;
using namespace MiKTeX::Core;
using namespace MiKTeX::Test;
using namespace MiKTeX::Util;

BEGIN_TEST_SCRIPT("creator-1");

BEGIN_TEST_FUNCTION(1);
{
    shared_ptr<Creator> creator;
    TESTX(creator = Creator::New(ArchiveFileType::Tar));
    FileSet fileSet = {
        PathName(TEST_SOURCE_DIR) / "creator" / "testdata",
        "",
        {
            "a"
        }
    };
    TESTX(creator->Create(PathName(TEST_BINARY_DIR) / "creator" / "test.tar", { fileSet }));
}
END_TEST_FUNCTION();

BEGIN_TEST_FUNCTION(2);
{
    FileStream outFile(File::Open(PathName(TEST_BINARY_DIR)  / "creator" / "largefile", FileMode::Create, FileAccess::Write, false));
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist(0, 255);
    for (int i = 0; i < 1000; ++i)
    {
        char buffer[1000];
        for (int j = 0; j < 1000; ++j)
        {
            buffer[j] = static_cast<char>(dist(gen));
        }
        outFile.Write(buffer, sizeof(buffer));
    }
    outFile.Close();
    shared_ptr<Creator> creator;
    TESTX(creator = Creator::New(ArchiveFileType::TarBzip2));
    FileSet fileSet = {
        PathName(TEST_BINARY_DIR) / "creator",
        "foo/bar/",
        {
            "largefile"
        }
    };
    TESTX(creator->Create(PathName(TEST_BINARY_DIR) / "creator" / "test.tar.bz2", { fileSet }));
}
END_TEST_FUNCTION();

BEGIN_TEST_PROGRAM();
{
    CALL_TEST_FUNCTION(1);
    CALL_TEST_FUNCTION(2);
}
END_TEST_PROGRAM();

END_TEST_SCRIPT();

RUN_TEST_SCRIPT ();
