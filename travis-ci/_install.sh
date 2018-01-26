cd "${TRAVIS_BUILD_DIR}/build"

if [ "${TRAVIS_OS_NAME}" = "osx" ]; then
    miktex_bin="${TRAVIS_BUILD_DIR}/install/MiKTeX Console.app/Contents/bin"
    miktex_home="$HOME/Library/Application Support/MiKTeX"
else
    miktex_bin="/usr/local/bin"
    miktex_home="$HOME/.miktex"
fi

sudomiktex() {
    set +e
    sudo MIKTEX_SHAREDSETUP=t "$miktex_bin"/$1 $2 $3 $4 $5 $6 $7 $8 $9
    local status=$?
    if [ $status -ne 0 ]; then
	if [ -d "$miktex_home/texmfs/data/miktex/log" ]; then
	    cd "$miktex_home/texmfs/data/miktex/log"
	    grep FATAL *
	fi
	if [ -d "/var/log/miktex" ]; then
	    cd "/var/log/miktex"
	    grep FATAL *
	fi
    fi
    set -e
    return $status
}

sudo make install

sudomiktex miktexsetup --shared=yes finish
sudomiktex mpm --admin --package-level=basic --upgrade

rm -fr "${miktex_home}"
