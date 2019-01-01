#!/usr/bin/env sh

dist=$MIKTEX_LINUX_DIST
tag=$MIKTEX_LINUX_DIST_CODE_NAME

if [ -d "${HOME}/builds/$dist-$tag/sandbox/miktex/log" ]; then
    (cd "${HOME}/builds/$dist-$tag/sandbox/miktex/log"; grep FATAL *)
fi

if [ -f "${HOME}/builds/$dist-$tag/Testing/Temporary/LastTest.log" ]; then
    cat "${HOME}/builds/$dist-$tag/Testing/Temporary/LastTest.log"
fi

if [ -f "${HOME}/builds/$dist-$tag/Libraries/MiKTeX/Core/test/sandbox/localtexmf/miktex/log/coretest.log" ]; then
    cat "${HOME}/builds/$dist-$tag/Libraries/MiKTeX/Core/test/sandbox/localtexmf/miktex/log/coretest.log"
fi

if [ -f "$HOME/tests/$dist-$tag/test/Testing/Temporary/LastTest.log" ]; then
    cat "$HOME/tests/$dist-$tag/test/Testing/Temporary/LastTest.log"
fi

if [ -d "$HOME/tests/$dist-$tag/test/logfiles" ]; then
    (cd "$HOME/tests/$dist-$tag/test/logfiles"; cat *)
fi
