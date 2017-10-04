#!/usr/bin/env sh

#set -ev

if [ -d ~/.miktex/texmfs/data/miktex/log ]; then
    cd ~/.miktex/texmfs/data/miktex/log
    grep FATAL *
fi

if [ -f "${TRAVIS_BUILD_DIR}/miktex-testing/build/Testing/Temporary/LastTest.log" ]; then
    cat "${TRAVIS_BUILD_DIR}/miktex-testing/build/Testing/Temporary/LastTest.log"
fi

for core in /tmp/*.core; do
    basename=$(basename "${core}")
    name=${basename%.*}
    gdb --batch --quiet -ex "bt" "/usr/local/bin/${name}" "${core}"
done
