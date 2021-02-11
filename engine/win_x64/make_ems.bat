@echo off
cls

setlocal

set P=%cd%\..

echo -------- Compiling source

set ExecutableName=Application

set SourceApplication=%P%\source\engine\unity_main.cpp

set Include_stb=-I %P%\externals\stb
set Source_stb=%P%\externals\stb\stb_implementation.cpp

set Include_SDL=-I %P%\externals\SDL2-2.0.10\include

set Include_cJSON=-I %P%\externals\cJSON\
set Source_cJSON=%P%\externals\cJSON\cJSON.c

set Include_fast_obj=-I %P%\externals\fast_obj\
set Source_fast_obj=%P%\externals\fast_obj\fast_obj_implementation.cpp

set Include_klib=-I %P%\externals\klib\

set CompilerFlags=-o %Executablename%.html -std=c++17 -O2

set Defines=-DNDEBUG -DLIB_STB -DLIB_CJSON -DLIB_FAST_OBJ -DPLATFORM_LAYER_SDL -DRENDERER_OPENGL3
set EmscriptenDefines=-s USE_SDL=2 -s FULL_ES3=1 -s ALLOW_MEMORY_GROWTH=1 --preload-file %P%\data@\data

pushd %P%\win_x64\ems

echo -- using em++
em++ %CompilerFlags%                                                                        ^
    %Defines%                                                                               ^
    %EmscriptenDefines%                                                                     ^
    %Include_SDL% %Include_stb% %Include_cJSON% %Include_fast_obj% %Include_klib%           ^
    %SourceApplication% %Source_stb% %Source_cJSON% %Source_fast_obj%

echo.
echo -------- Finished compiling source

popd
endlocal
