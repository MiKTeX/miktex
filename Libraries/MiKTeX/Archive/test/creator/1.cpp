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

#include <miktex/Core/Test>

#include <miktex/Archive/Creator>

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
    shared_ptr<Creator> creator;
    TESTX(creator = Creator::New(ArchiveFileType::TarBzip2));
    FileSet fileSet = {
        PathName(TEST_SOURCE_DIR) / "creator" / "testdata" / "a",
        "foo/bar/",
        {
            "."
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
