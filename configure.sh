echo Building the autotools build system for the project with autoreconf...
autoreconf --install
echo Creating the final autotools makefile...
./configure CXX=${PWD}/scripts/clang_wrapper.sh
