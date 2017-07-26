cd "${TRAVIS_BUILD_DIR}/build"

if [ "${TRAVIS_OS_NAME}" = "linux" ]; then
    sudo=sudo
fi

${sudo} make install

${sudo} initexmf --admin --disable-installer --update-fndb --mklinks
${sudo} mpm --admin --package-level=basic --upgrade
${sudo} initexmf --admin --mkmaps

${sudo} rm -fr ~/.miktex
