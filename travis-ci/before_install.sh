#!/usr/bin/env sh

set -ev

if [ "${TRAVIS_OS_NAME}" = "osx" ]; then
    brew update

    brew outdated cmake || brew upgrade cmake
    brew outdated pkg-config || brew upgrade pkg-config

    ( brew ls -1 | grep -w dos2unix ) || brew install dos2unix
    ( brew ls -1 | grep -w fop ) || brew install fop
    ( brew ls -1 | grep -w pandoc ) || brew install pandoc

    brew install harfbuzz --with-graphite2
    
    ( brew ls -1 | grep -w apr ) || brew install apr
    ( brew ls -1 | grep -w apr-util ) || brew install apr-util
    ( brew ls -1 | grep -w cairo ) || brew install cairo
    ( brew ls -1 | grep -w fontconfig ) || brew install fontconfig
    ( brew ls -1 | grep -w freetype ) || brew install freetype
    ( brew ls -1 | grep -w fribidi ) || brew install fribidi
    ( brew ls -1 | grep -w gd ) || brew install gd
    ( brew ls -1 | grep -w glib ) || brew install glib
    ( brew ls -1 | grep -w gmp ) || brew install gmp
    ( brew ls -1 | grep -w graphite2 ) || brew install graphite2
    ( brew ls -1 | grep -w hunspell ) || brew install hunspell
    ( brew ls -1 | grep -w icu4c ) || brew install icu4c
    ( brew ls -1 | grep -w jpeg ) || brew install jpeg
    ( brew ls -1 | grep -w libffi ) || brew install libffi
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
