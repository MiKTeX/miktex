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
    sudo ulimit -c unlimited -S
    sudo sysctl -w "kernel.core_pattern=/tmp/%e.core"
fi

make test
