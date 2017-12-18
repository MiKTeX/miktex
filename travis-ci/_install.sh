cd "${TRAVIS_BUILD_DIR}/build"

if [ "${TRAVIS_OS_NAME}" = "osx" ]; then
    miktex_bin="${TRAVIS_BUILD_DIR}/build-install/MiKTeX.app/Contents/bin"
    miktex_home="$HOME/Library/Application Support/MiKTeX"
else
    miktex_bin="/usr/local/bin"
    miktex_home="$HOME/.miktex"
fi

sudomiktex() {
    set +e
    sudo MIKTEX_SHAREDSETUP=t "${miktex_bin}/$@"
    local status=$?
    if [ $status -ne 0 ]; then
	if [ -d "$miktex_home/texmfs/data/miktex/log" ]; then
	    cd "$miktex_home/texmfs/data/miktex/log"
	    grep FATAL *
	fi
    fi
    set -e
    return $status
}

sudo make install

sudomiktex initexmf --admin --disable-installer --update-fndb --mklinks
sudomiktex mpm --admin --package-level=basic --upgrade
sudomiktex initexmf --admin --mkmaps

rm -fr "${miktex_home}"
