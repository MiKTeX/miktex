cd "${TRAVIS_BUILD_DIR}/build"

if [ "${TRAVIS_OS_NAME}" = "linux" ]; then
    sudo=sudo
fi

function runmiktex () {
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

runmiktex ${sudo} initexmf --admin --disable-installer --update-fndb --mklinks
runmiktex ${sudo} mpm --admin --package-level=basic --upgrade
runmiktex ${sudo} initexmf --admin --mkmaps

${sudo} rm -fr ~/.miktex
