if [ "${TRAVIS_OS_NAME}" = "linux" ]; then
    cmake="${TRAVIS_BUILD_DIR}/mycmake/bin/cmake"
else
    cmake=cmake
fi

if [ "${TRAVIS_OS_NAME}" = "linux" ]; then
    ulimit -Sc unlimited
    sudo bash -c "echo '/tmp/%e.core' > /proc/sys/kernel/core_pattern"
fi

cd "${TRAVIS_BUILD_DIR}"

git clone https://github.com/MiKTeX/miktex-testing

cd miktex-testing
mkdir build
cd build

${cmake} -DMIKTEX_BINARY_DIR="$miktex_bin" ..

make test
