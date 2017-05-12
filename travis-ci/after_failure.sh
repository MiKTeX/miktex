#!/usr/bin/env sh

#set -ev

cat "${TRAVIS_BUILD_DIR}/miktex-testing/build/Testing/Temporary/LastTest.log"
cat ~/.miktex/texmfs/data/miktex/log/miktex-pdftex.log
grep FATAL ~/.miktex/texmfs/data/miktex/log/*
