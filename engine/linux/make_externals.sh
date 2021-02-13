#!/bin/bash
echo -------- Generating external libraries

cd $(dirname $0)
externals=$PWD/../externals

LibraryPath=${externals}/lib
mkdir -p ${LibraryPath}

Source_stb=${externals}/stb/stb_implementation.cpp
Library_stb=stb.a
Compiled_stb=stb.obj

Source_cJSON=${externals}/cJSON/cJSON.c
Library_cJSON=cjson.a
Compiled_cJSON=cjson.obj

Source_fast_obj=${externals}/fast_obj/fast_obj_implementation.cpp
Library_fast_obj=fast_obj.a
Compiled_fast_obj=fast_obj.obj

pushd ${LibraryPath} > /dev/null

echo -------- stb

g++ -O2 -c -o ${LibraryPath}/${Compiled_stb} ${Source_stb}
ar rcs lib${Library_stb} ${Compiled_stb}

echo -------- cJSON

g++ -O2 -c -o ${LibraryPath}/${Compiled_cJSON} ${Source_cJSON}
ar rcs lib${Library_cJSON} ${Compiled_cJSON}

echo -------- fast_obj

g++ -O2 -c -o ${LibraryPath}/${Compiled_fast_obj} ${Source_fast_obj}
ar rcs lib${Library_fast_obj} ${Compiled_fast_obj}

echo -------- Finished generating external libraries

popd > /dev/null
