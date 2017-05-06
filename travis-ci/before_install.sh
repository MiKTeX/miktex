#!/usr/bin/env sh

set -ev

if [ "${TRAVIS_OS_NAME}" = "osx" ]; then
    brew update

    brew outdated cmake || brew upgrade cmake
    brew outdated pkg-config || brew upgrade pkg-config

    ( brew ls -1 | grep dos2unix ) || brew install dos2unix
    ( brew ls -1 | grep fop ) || brew install fop
    ( brew ls -1 | grep pandoc ) || brew install pandoc

    ( brew ls -1 | grep apr ) || brew install apr
    ( brew ls -1 | grep apr ) || brew install apr-util
    ( brew ls -1 | grep cairo ) || brew install cairo
    ( brew ls -1 | grep fontconfig ) || brew install fontconfig
    ( brew ls -1 | grep freetype ) || brew install freetype
    ( brew ls -1 | grep fribidi ) || brew install fribidi
    ( brew ls -1 | grep gd ) || brew install gd
    ( brew ls -1 | grep glib ) || brew install glib
    ( brew ls -1 | grep gmp ) || brew install gmp
    ( brew ls -1 | grep graphite2 ) || brew install graphite2
    ( brew ls -1 | grep harfbuzz ) || brew install harfbuzz --with-graphite2
    ( brew ls -1 | grep hunspell ) || brew install hunspell
    ( brew ls -1 | grep icu4c ) || brew install icu4c
    ( brew ls -1 | grep jpeg ) || brew install jpeg
    ( brew ls -1 | grep libffi ) || brew install libffi
    ( brew ls -1 | grep libpng ) || brew install libpng
    ( brew ls -1 | grep libtiff ) || brew install libtiff
    ( brew ls -1 | grep libzzip ) || brew install libzzip
    ( brew ls -1 | grep log4cxx ) || brew install log4cxx
    ( brew ls -1 | grep mpfr ) || brew install mpfr
    ( brew ls -1 | grep openssl ) || brew install openssl
    ( brew ls -1 | grep pixman ) || brew install pixman
    ( brew ls -1 | grep poppler ) || brew install poppler
    ( brew ls -1 | grep popt ) || brew install popt
    ( brew ls -1 | grep potrace ) || brew install potrace
    ( brew ls -1 | grep uriparser ) || brew install uriparser
    ( brew ls -1 | grep xz ) || brew install xz
fi
