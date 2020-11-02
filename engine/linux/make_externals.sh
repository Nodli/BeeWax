#!/bin/bash

clear
echo "-------- Generating static libraries"

cd $(dirname $0)

echo -n "STB..."

CODEPATH=$PWD/../externals/stb
LIBPATH=$PWD/../externals/stb/static_library/linux

mkdir -p ${LIBPATH}
cd ${LIBPATH}

LibraryName=stb.a
IntermName=stb.o

g++ -c -o ${LIBPATH}/${IntermName} ${CODEPATH}/stb_libraries.cpp
ar rcs lib${LibraryName} ${IntermName}

echo " Done"
