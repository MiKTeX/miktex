#!/usr/bin/env sh

set -e
set -v

if [ "${TRAVIS_OS_NAME}" = "linux" ]; then
    mkdir "${TRAVIS_BUILD_DIR}/mycmake"
    cmakeurl=https://cmake.org/files/v3.8/cmake-3.8.1-Linux-x86_64.tar.gz
    wget -O - ${cmakeurl} | tar --strip-components=1 -xz -C "${TRAVIS_BUILD_DIR}/mycmake"
    
    sudo apt-get -qq update

    sudo apt-get install -y gdb

    sudo apt-get install -y pkg-config

    sudo apt-get install -y dpkg-dev

    sudo apt-get install -y pandoc
    sudo apt-get install -y xsltproc

    sudo apt-get install -y fontconfig
    sudo apt-get install -y ghostscript
    
    sudo apt-get install -y libapr1-dev
    sudo apt-get install -y libaprutil1-dev
    sudo apt-get install -y libcairo2-dev
    sudo apt-get install -y libffi-dev
    sudo apt-get install -y libfontconfig1-dev
    sudo apt-get install -y libfreetype6-dev
    sudo apt-get install -y libfribidi-dev
    sudo apt-get install -y libgd-dev
    sudo apt-get install -y libgmp-dev
    sudo apt-get install -y libgraphite2-dev
    sudo apt-get install -y libhunspell-dev
    sudo apt-get install -y libicu-dev
    sudo apt-get install -y libjpeg-dev
    sudo apt-get install -y liblog4cxx10-dev
    sudo apt-get install -y liblzma-dev
    sudo apt-get install -y libmpfr-dev
    sudo apt-get install -y libmspack-dev
    sudo apt-get install -y libpixman-1-dev
    sudo apt-get install -y libpng-dev
    sudo apt-get install -y libpopt-dev
    sudo apt-get install -y libpotrace-dev
    sudo apt-get install -y libssl-dev
    sudo apt-get install -y libtiff5-dev
    sudo apt-get install -y liburiparser-dev
    sudo apt-get install -y libzzip-dev

    if [ -z $MIKTEX_BUILD_HARFBUZZ ]; then
        sudo apt-get install -y libharfbuzz-dev
    fi

    if [ -z $MIKTEX_BUILD_POPPLER ]; then
	sudo apt-get install -y poppler-utils
	sudo apt-get install -y libpoppler-dev
	sudo apt-get install -y libpoppler-private-dev
    fi

elif [ "${TRAVIS_OS_NAME}" = "osx" ]; then
    brew update

    brew outdated cmake || brew upgrade cmake
    brew outdated pkg-config || brew upgrade pkg-config

    ( brew ls -1 | grep -w gdb ) || brew install gdb

    ( brew ls -1 | grep -w pandoc ) || brew install pandoc

    ( brew ls -1 | grep -w ghostscript ) || brew install ghostscript
    ( brew ls -1 | grep -w md5sha1sum ) || brew install md5sha1sum

    ( brew ls -1 | grep -w icu4c ) || brew install icu4c
    
    ( brew ls -1 | grep -w cairo ) || brew install cairo
    ( brew ls -1 | grep -w fontconfig ) || brew install fontconfig
    ( brew ls -1 | grep -w freetype ) || brew install freetype
    ( brew ls -1 | grep -w fribidi ) || brew install fribidi
    ( brew ls -1 | grep -w gd ) || brew install gd
    ( brew ls -1 | grep -w gmp ) || brew install gmp
    ( brew ls -1 | grep -w graphite2 ) || brew install graphite2
    ( brew ls -1 | grep -w hunspell ) || brew install hunspell
    ( brew ls -1 | grep -w jpeg ) || brew install jpeg
    ( brew ls -1 | grep -w libpng ) || brew install libpng
    ( brew ls -1 | grep -w libtiff ) || brew install libtiff
    ( brew ls -1 | grep -w libzzip ) || brew install libzzip
    ( brew ls -1 | grep -w log4cxx ) || brew install log4cxx
    ( brew ls -1 | grep -w mpfr ) || brew install mpfr
    ( brew ls -1 | grep -w openssl ) || brew install openssl
    ( brew ls -1 | grep -w pixman ) || brew install pixman
    ( brew ls -1 | grep -w poppler ) || brew install poppler
    ( brew ls -1 | grep -w popt ) || brew install popt
    ( brew ls -1 | grep -w potrace ) || brew install potrace
    ( brew ls -1 | grep -w uriparser ) || brew install uriparser
    ( brew ls -1 | grep -w xz ) || brew install xz
fi
