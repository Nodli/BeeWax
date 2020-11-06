#!/bin/bash

cd $(dirname $0)

mkdir -p bin/
cp -r -u ../data bin/

P=$PWD/..

ExecutableName=Application

SourceApplication=${P}/source/engine/unity_main.cpp

# OpenGL
IncludeGL3W="-I ${P}/externals/gl3w/include/"
SourceGL3W=$P/externals/gl3w/src/gl3w.c
LinkOpenGL=-lGL

# Vulkan
IncludeVulkan="-I ${P}/externals/vulkan/"
LinkVulkan=-lvulkan

IncludeSTB="-I ${P}/externals/stb/"
LinkSTB="-L ${P}/externals/stb/static_library/linux -lstb"

IncludeLinkSDL=$(sdl2-config --cflags --libs)
LinkDynamicLinking=-ldl

CompilerFlags="-o ${ExecutableName} -std=c++17"
OptimizationFlags="-fno-exceptions"
WarningFlags="-Wall -Wextra -Werror"
WarningExtraFlags="-Wsign-compare -Wsign-conversion -Wconversion"
DebugFlags="-g -DDEBUG_BUILD"
SanitizeFlags="-fsanitize=address"

cd bin/

# ---- SDL OpenGL STB
EXIT_CODE= g++                                              \
$CompilerFlags                                              \
$OptimizationFlags                                          \
-DLIB_STB -DPLATFORM_LAYER_SDL -DRENDERER_OPENGL3           \
$SourceApplication $SourceGL3W                              \
$IncludeGL3W $IncludeSTB                                    \
$LinkSTB $IncludeLinkSDL $LinkOpenGL $LinkDynamicLinking    \
$DebugFlags                                                 \
#$SanitizeFlags                                              \
#$WarningFlags $WarningExtraFlags                            \

exit $EXIT_CODE
