cd "${TRAVIS_BUILD_DIR}/build"

if [ "${TRAVIS_OS_NAME}" = "linux" ]; then
    sudo=sudo
fi

${sudo} make install

${sudo} mpm --admin --update-db
${sudo} initexmf --admin --set-config-value [MPM]AutoInstall=1
${sudo} initexmf --admin --update-fndb
${sudo} initexmf --admin --mklinks
${sudo} mpm --admin --package-level=basic --upgrade
${sudo} initexmf --admin --mkmaps

${sudo} rm -fr ~/.miktex
