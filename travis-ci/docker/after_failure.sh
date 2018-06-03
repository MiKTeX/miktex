#!/usr/bin/env sh

dist=$MIKTEX_LINUX_DIST
tag=$MIKTEX_LINUX_DIST_CODE_NAME

if [ -f "$HOME/tests/$dist-$tag/test/Testing/Temporary/LastTest.log" ]; then
    cat "$HOME/tests/$dist-$tag/test/Testing/Temporary/LastTest.log"
else
    echo log file does not exist: "$HOME/tests/$dist-$tag/test/Testing/Temporary/LastTest.log"
fi

if [ -d "$HOME/tests/$dist-$tag/test/logfiles" ]; then
    (cd "$HOME/tests/$dist-$tag/test/logfiles"; cat *)
else
    echo log directory does not exist: "$HOME/tests/$dist-$tag/test/logfiles"
fi
