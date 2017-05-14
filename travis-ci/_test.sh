cd "${TRAVIS_BUILD_DIR}"

git clone https://github.com/MiKTeX/miktex-testing
cd miktex-testing
mkdir build
cd build

if [ "${TRAVIS_OS_NAME}" = "linux" ]; then
    cmake="${TRAVIS_BUILD_DIR}/mycmake/bin/cmake"
else
    cmake=cmake
fi

${cmake} ..

if [ "${TRAVIS_OS_NAME}" = "linux" ]; then
    ulimit -c unlimited -S
    sudo bash -c "echo '/tmp/%e.core' > /proc/sys/kernel/core_pattern"
fi

make test
