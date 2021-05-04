#!/bin/bash

echo "-------- Bash script arguments"

ApplicationUnity=$1
ApplicationSettings=$2

echo -------- Path

ApplicationDirectory=$PWD

pushd $(dirname $0) > /dev/null
ProjectDirectory=$PWD
popd > /dev/null

RootDirectory=$ProjectDirectory/../..
EngineDirectory=$RootDirectory/engine
CommonDirectory=$RootDirectory/common

echo "Application:  " $ApplicationDirectory
echo "Project:      " $ProjectDirectory
echo "Root:         " $RootDirectory
echo "Engine:       " $EngineDirectory

echo -------- Compiling data

BinDirectory=$ApplicationDirectory/bin
mkdir -p $BinDirectory

$ProjectDirectory/make_data.sh $BinDirectory/data $EngineDirectory/data
$ProjectDirectory/make_data.sh $BinDirectory/data $ApplicationDirectory/data

echo -------- Compiling source

ExecutableName=Application

Source_Engine=$EngineDirectory/source/unity.cpp
Include_Common="-I $CommonDirectory/source"

Library_SDL=$(sdl2-config --cflags --libs)

Library_OpenGL="-lGL"

Include_gl3w="-I $RootDirectory/externals/gl3w/include/"
Source_gl3w=$RootDirectory/externals/gl3w/src/gl3w.c

Include_stb="-I $RootDirectory/externals/stb/"
Library_stb="-L $RootDirectory/externals/lib -lstb"

Include_cJSON="-I $RootDirectory/externals/cJSON/"
Library_cJSON="-L $RootDirectory/externals/lib -lcjson"

Include_fast_obj="-I $RootDirectory/externals/fast_obj/"
Library_fast_obj="-L $RootDirectory/externals/lib -lfast_obj"

Include_klib="-I $RootDirectory/externals/klib/"

Include_ImGui="-I $RootDirectory/externals/imgui/"
Library_ImGui="-L $RootDirectory/externals/lib -limgui"

CompilerFlags="-o $ExecutableName -std=c++17"
LinkerFlags="-ldl"
OptimizationFlags="-fno-rtti -fno-exceptions"

DebugFlags="-g -DDEBUG"
#AdressSanitizer="-fsanitize=address"
#WarningFlags="-Wall -Wextra -Werror"
#WarningExtraFlags="-Wsign-compare -Wsign-conversion -Wconversion"

EngineDefines="-DLIB_STB -DLIB_CJSON -DLIB_FAST_OBJ -DPLATFORM_LAYER_SDL -DRENDERER_OPENGL3 -DDEVELOPPER_MODE"

if [[ $ApplicationUnity ]];
then
    ApplicationUnityInclude=$ApplicationDirectory/$ApplicationUnity
    echo "ApplicationUnity:         " $ApplicationUnityInclude
    PreEngineDefines="-DAPPLICATION_UNITY=\"$ApplicationUnityInclude\""
fi

if [[ $ApplicationSettings ]];
then
    ApplicationSettingsInclude=$ApplicationDirectory/$ApplicationSettings
    echo "ApplicationSettings:      " $ApplicationSettingsInclude
    PostEngineDefines="-DAPPLICATION_SETTINGS=\"$ApplicationSettingsInclude\""
fi

if ! [[ -v DebugFlags ]];
then
    echo -- release mode
    DebugFlags=-DNDEBUG
else
    echo -- debug mode
fi

pushd $BinDirectory > /dev/null

g++                                                                                         \
$CompilerFlags                                                                              \
$OptimizationFlags                                                                          \
$DebugFlags                                                                                 \
$AdressSanitizer                                                                            \
$WarningFlags                                                                               \
$WarningExtraFlags                                                                          \
$EngineDefines                                                                              \
$PreEngineDefines $PostEngineDefines                                                        \
$Include_gl3w $Include_stb $Include_cJSON $Include_fast_obj $Include_klib $Include_ImGui    \
$Include_Common                                                                             \
$Source_Engine $Source_gl3w                                                                 \
$LinkerFlags                                                                                \
$Library_SDL $Library_OpenGL $Library_cJSON $Library_fast_obj $Library_ImGui $Library_stb

ReturnCode=$?

echo -------- Finished compiling source

popd > /dev/null

exit $ReturnCode
