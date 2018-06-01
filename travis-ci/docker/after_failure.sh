#!/usr/bin/env sh

set -v

if [ -f "~/tests/*/test/Testing/Temporary/LastTest.log" ]; then
    cat "~/tests/*/test/Testing/Temporary/LastTest.log"
fi

if [ -d "~/tests/*/test/logfiles" ]; then
    (cd "~/tests/*/test/logfiles"; cat *)
fi
