#!/bin/bash

echo -------- Generating external libraries

pushd $(dirname $0) > /dev/null
RootDirectory=$PWD/../..
ExternalsDirectory=$RootDirectory/externals
popd > /dev/null

LibraryPath=$ExternalsDirectory/lib
mkdir -p $LibraryPath

Source_stb=$ExternalsDirectory/stb/stb_source.cpp
Library_stb=stb.a
Compiled_stb=stb.obj

Source_cJSON=$ExternalsDirectory/cJSON/cJSON.c
Library_cJSON=cjson.a
Compiled_cJSON=cjson.obj

Source_fast_obj=$ExternalsDirectory/fast_obj/fast_obj_source.cpp
Library_fast_obj=fast_obj.a
Compiled_fast_obj=fast_obj.obj

Source_ImGui=$ExternalsDirectory/imgui/imgui_source.cpp
Library_ImGui=imgui.a
Compiled_ImGui=imgui.obj
External_ImGui="-I $ExternalsDirectory/stb -I $ExternalsDirectory/gl3w/include $(sdl2-config --cflags)"

#DebugFlags="-g -DDEBUG"
if ! [[ -v DebugFlags ]];
then
    echo -- release mode
    DebugFlags=-DNDEBUG
else
    echo -- debug mode
fi

pushd $LibraryPath > /dev/null

echo -------- stb

g++ -O2 -c $DebugFlags -o $LibraryPath/$Compiled_stb $Source_stb
ar rcs lib$Library_stb $Compiled_stb

echo -------- cJSON

g++ -O2 -c $DebugFlags -o $LibraryPath/$Compiled_cJSON $Source_cJSON
ar rcs lib$Library_cJSON $Compiled_cJSON

echo -------- fast_obj

g++ -O2 -c $DebugFlags -o $LibraryPath/$Compiled_fast_obj $Source_fast_obj
ar rcs lib$Library_fast_obj $Compiled_fast_obj

echo -------- ImGui

g++ -O2 -c $DebugFlags -o $LibraryPath/$Compiled_ImGui $Source_ImGui $External_ImGui
ar rcs lib$Library_ImGui $Compiled_ImGui

echo -------- Finished generating external libraries

popd > /dev/null
