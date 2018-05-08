#!/usr/bin/env sh

set -v

miktex_home="$HOME/Library/Application Support/MiKTeX"

if [ -d "$miktex_home/texmfs/data/miktex/log" ]; then
    (cd "$miktex_home/texmfs/data/miktex/log"; cat *)
fi

if [ -d "${TRAVIS_BUILD_DIR}/build/sandbox/miktex/log" ]; then
    (cd "${TRAVIS_BUILD_DIR}/build/sandbox/miktex/log"; grep FATAL *)
fi

if [ -f "${TRAVIS_BUILD_DIR}/miktex-testing/build/Testing/Temporary/LastTest.log" ]; then
    cat "${TRAVIS_BUILD_DIR}/miktex-testing/build/Testing/Temporary/LastTest.log"
fi

for core in /tmp/*.core; do
    basename=$(basename "${core}")
    name=${basename%.*}
    gdb --batch --quiet -ex "bt" "/usr/local/bin/${name}" "${core}"
done
