cd "${TRAVIS_BUILD_DIR}/build"

if [ "${TRAVIS_OS_NAME}" = "linux" ]; then
    sudo=sudo
fi

runmiktex() {
    set +e
    "$@"
    local status=$?
    if [ $status -ne 0 ]; then
	if [ -d ~/.miktex/texmfs/data/miktex/log ]; then
	    cd ~/.miktex/texmfs/data/miktex/log
	    grep FATAL *
	fi
    fi
    set -e
    return $status
}

${sudo} make install

if [ "${TRAVIS_OS_NAME}" = "osx" ]; then
    export PATH=${TRAVIS_BUILD_DIR}/build-install/MiKTeX.app/Contents/bin;$PATH
fi

runmiktex ${sudo} initexmf --admin --disable-installer --update-fndb --mklinks
runmiktex ${sudo} mpm --admin --package-level=basic --upgrade
runmiktex ${sudo} initexmf --admin --mkmaps

if [ "${TRAVIS_OS_NAME}" = "linux" ]; then
    rm -fr ~/.miktex
else
    rm -fr "~/Library/Application Support/MiKTeX"
fi
