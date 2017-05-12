mkdir "${TRAVIS_BUILD_DIR}/testing"
cd "${TRAVIS_BUILD_DIR}/testing"

git clone https://github.com/MiKTeX/miktex-testing

mkdir test
cd test

if [ "${TRAVIS_OS_NAME}" = "linux" ]; then
    cmake="${TRAVIS_BUILD_DIR}/mycmake/bin/cmake"
else
    cmake=cmake
fi

${cmake} ..

make test
