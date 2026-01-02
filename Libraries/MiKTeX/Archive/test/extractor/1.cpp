/**
 * @file test/extractor/1.cpp
 * @author Christian Schenk
 * @brief Extractor test
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

#include <miktex/Archive/Extractor>
#include <miktex/Core/MD5>

using namespace std;

using namespace MiKTeX::Archive;
using namespace MiKTeX::Core;
using namespace MiKTeX::Test;
using namespace MiKTeX::Util;

BEGIN_TEST_SCRIPT("extractor-1");

BEGIN_TEST_FUNCTION(1);
{
    shared_ptr<Extractor> extractor;
    TESTX(extractor = Extractor::New(ArchiveFileType::TarBzip2));
    TESTX(extractor->Extract(PathName(TEST_SOURCE_DIR) / "extractor" / "test.tar.bz2", PathName(TEST_BINARY_DIR) / "extractor" / "out", true));
    TEST(MD5::FromFile(PathName(TEST_BINARY_DIR) / "extractor" / "out" / "a" / "hello") == MD5::Parse("78016cea74c298162366b9f86bfc3b16"));
}
END_TEST_FUNCTION();

BEGIN_TEST_PROGRAM();
{
  CALL_TEST_FUNCTION(1);
}
END_TEST_PROGRAM();

END_TEST_SCRIPT();

RUN_TEST_SCRIPT ();
