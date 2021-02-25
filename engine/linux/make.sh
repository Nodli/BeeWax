#!/bin/bash
clear
cd $(dirname $0)
P=$PWD/..

./make_data.sh bin data

echo -------- Compiling source

ExecutableName=Application

SourceApplication=${P}/source/engine/unity_main.cpp

Include_gl3w="-I ${P}/externals/gl3w/include/"
Source_gl3w=$P/externals/gl3w/src/gl3w.c

Library_OpenGL=-lGL

Include_stb="-I ${P}/externals/stb/"
Library_stb="-L ${P}/externals/lib/ -lstb"

Library_SDL=$(sdl2-config --cflags --libs)

Include_cJSON="-I ${P}/externals/cJSON/"
Library_cJSON="-L ${P}/externals/lib/ -lcjson"

Include_fast_obj="-I ${P}/externals/fast_obj/"
Library_fast_obj="-L ${P}/externals/lib/ -lfast_obj"

Include_klib="-I ${P}/externals/klib/"

Include_ImGui="-I ${P}/externals/imgui/"
Library_ImGui="-L ${P}/externals/lib/ -limgui"
Source_ImGui=${P}/externals/imgui/*.cpp

CompilerFlags="-o ${ExecutableName} -std=c++17"
LinkerFlags="-ldl"
OptimizationFlags="-fno-rtti -fno-exceptions"
DebugFlags="-g"
#AdressSanitizer="-fsanitize=address"

#WarningFlags="-Wall -Wextra -Werror"
#WarningExtraFlags="-Wsign-compare -Wsign-conversion -Wconversion"

Defines="-DLIB_STB -DLIB_CJSON -DLIB_FAST_OBJ -DPLATFORM_LAYER_SDL -DRENDERER_OPENGL3 -DDEVELOPPER_MODE"
if ! [[ -v DebugFlags ]];
then
    echo -- release mode
    DebugFlags=-DNDEBUG
else
    echo -- debug mode
fi

pushd ${P}/linux/bin > /dev/null

g++                                                                                         \
$CompilerFlags                                                                              \
$OptimizationFlags                                                                          \
$DebugFlags                                                                                 \
$AdressSanitizer                                                                            \
$WarningFlags                                                                               \
$WarningExtraFlags                                                                          \
$Defines                                                                                    \
$Include_gl3w $Include_stb $Include_cJSON $Include_fast_obj $Include_klib $Include_ImGui    \
$SourceApplication $Source_gl3w                                                             \
$LinkerFlags                                                                                \
$Library_SDL $Library_OpenGL $Library_stb $Library_cJSON $Library_fast_obj $Library_ImGui

ReturnCode=$?

echo -------- Finished compiling source

popd > /dev/null

exit $ReturnCode
