#!/usr/bin/env sh

#set -ev

if [ -e "${TRAVIS_BUILD_DIR}/miktex-testing/build/Testing/Temporary/LastTest.log" ]; then
   curl -sT "${TRAVIS_BUILD_DIR}/miktex-testing/build/Testing/Temporary/LastTest.log" chunk.io
fi

if [ -d ~/.miktex/texmfs/data/miktex/log ]; then
    cd ~/.miktex/texmfs/data/miktex/log
    tar -cJf logs.tar.xz *.log
    curl -sT logs.tar.xz chunk.io
fi

if [ -d "${TRAVIS_BUILD_DIR}/miktex-testing/build/pdftex" ]; then
    ls -l "${TRAVIS_BUILD_DIR}/miktex-testing/build/pdftex"
fi

if [ -e /tmp/miktex-makepk.core ]; then
    gdb --batch --quiet -ex "bt" /usr/local/bin/miktex-makepk /tmp/miktex-makepk.core
fi
