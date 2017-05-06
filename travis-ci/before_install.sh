#!/usr/bin/env sh

set -ev

if [ "${TRAVIS_OS_NAME}" = "osx" ]; then
    brew update
    brew outdated cmake || brew upgrade cmake
    brew install apr
    brew install apr-util
    brew install cairo
    brew install fontconfig
    brew install freetype
    brew install fribidi
    brew install gd
    brew install glib
    brew install gmp
    brew install graphite2
    brew install harfbuzz --with-graphite2
    brew install hunspell
    brew install icu4c
    brew install jpeg
    brew install libffi
    brew install libpng
    brew install libtiff
    brew install libzzip
    brew install log4cxx
    brew install mpfr
    brew install openssl
    brew install pixman
    brew install poppler
    brew install popt
    brew install potrace
    brew install uriparser
    brew install xz
fi
