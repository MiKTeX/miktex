cd "${TRAVIS_BUILD_DIR}"

git clone https://github.com/MiKTeX/miktex-testing

cd miktex-testing
mkdir build
cd build

cmake -DMIKTEX_BINARY_DIR="$miktex_bin" ..

#export MIKTEX_TRACE=error
CTEST_OUTPUT_ON_FAILURE=1 make test
