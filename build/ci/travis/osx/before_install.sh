#!/usr/bin/env sh

set -e
set -v

brew update

brew outdated cmake || brew upgrade cmake
brew outdated pkg-config || brew upgrade pkg-config

( brew ls -1 | grep -w gdb ) || brew install gdb

( brew ls -1 | grep -w ghostscript ) || brew install ghostscript
( brew ls -1 | grep -w md5sha1sum ) || brew install md5sha1sum

( brew ls -1 | grep -w icu4c ) || brew install icu4c
    
( brew ls -1 | grep -w freetype ) || brew install freetype
( brew ls -1 | grep -w fribidi ) || brew install fribidi
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
( brew ls -1 | grep -w popt ) || brew install popt
( brew ls -1 | grep -w qt ) || brew install qt
( brew ls -1 | grep -w uriparser ) || brew install uriparser
( brew ls -1 | grep -w xz ) || brew install xz
