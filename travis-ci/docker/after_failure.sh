#!/usr/bin/env sh

set -v

dist=$MIKTEX_LINUX_DIST
tag=$MIKTEX_LINUX_DIST_CODE_NAME

if [ -f "~/tests/$dist-$tag/test/Testing/Temporary/LastTest.log" ]; then
    cat "~/tests/$dist-$tag/test/Testing/Temporary/LastTest.log"
fi

if [ -d "~/tests/$dist-$tag/test/logfiles" ]; then
    (cd "~/tests/$dist-$tag/test/logfiles"; cat *)
fi
