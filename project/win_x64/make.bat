@echo off
setlocal EnableDelayedExpansion

echo -------- Bat script arguments

set ApplicationUnity=%1
set ApplicationSettings=%2

echo -------- Path

set Applicationdirectory=%cd%

pushd %~dp0
set ProjectDirectory=%cd%
popd

set RootDirectory=%ProjectDirectory%\..\..
set EngineDirectory=%RootDirectory%\engine
set CommonDirectory=%RootDirectory%\common

echo "Application:  " %ApplicationDirectory%
echo "Project:      " %ProjectDirectory%
echo "Root:         " %RootDirectory%
echo "Engine:       " %EngineDirectory%

echo -------- Compiling data

set BinDirectory=%ApplicationDirectory%\bin
if not exist %BinDirectory% mkdir %BinDirectory%

call %ProjectDirectory%\make_data.bat %BinDirectory%\data %EngineDirectory%\data
call %ProjectDirectory%\make_data.bat %BinDirectory%\data %ApplicationDirectory%\data

xcopy %RootDirectory%\externals\SDL2-2.0.10\x64\SDL2.dll %BinDirectory% /E /Q /I /D /Y

echo -------- Compiling source

set PathPDB=%BinDirectory%\
set PathOBJ=%BinDirectory%\obj\
if not exist %PathOBJ% mkdir %PathOBJ%

set ExecutableName=Application

set Source_Engine=%EngineDirectory%\source\unity.cpp
set Include_Common=/I %CommonDirectory%\source

set Include_SDL=/I %RootDirectory%\externals\SDL2-2.0.10\include
set Library_SDL=/LIBPATH:%RootDirectory%\externals\SDL2-2.0.10\x64 SDL2.lib SDL2main.lib

set Library_OpenGL=opengl32.lib

set Include_gl3w=/I %RootDirectory%\externals\gl3w\include\
set Source_gl3w=%RootDirectory%\externals\gl3w\src\gl3w.c

set Include_stb=/I %RootDirectory%\externals\stb
set Library_stb=%RootDirectory%\externals\lib\stb.lib

set Include_cJSON=/I %RootDirectory%\externals\cJSON\
set Library_cJSON=%RootDirectory%\externals\lib\cjson.lib

set Include_fast_obj=/I %RootDirectory%\externals\fast_obj\
set Library_fast_obj=%RootDirectory%\externals\lib\fast_obj.lib

set Include_klib=/I %RootDirectory%\externals\klib\

set Include_ImGui=/I %RootDirectory%\externals\imgui
set Library_ImGui=%RootDirectory%\externals\lib\imgui.lib

set Library_win32=user32.lib gdi32.lib

set CompilerFlags=/nologo /Fe%Executablename% /FC /std:c++17 /cgthreads4 /Fo%PathOBJ%
set OptimizationFlags=/MT /GR- /EHsc /EHa- /Oi /O2

REM set DebugFlags=/Od /Zi /Fd%PathPDB% /DDEBUG
REM set AdressSanitizer=-fsanitize=address

set EngineDefines=/DLIB_STB /DLIB_CJSON /DLIB_FAST_OBJ /DPLATFORM_LAYER_SDL /DRENDERER_OPENGL3 /DDEVELOPPER_MODE

if not "%ApplicationUnity%" == "" (
    set ApplicationUnityInclude=%ApplicationDirectory%\%ApplicationUnity%
    echo "ApplicationUnity:         " !ApplicationUnityInclude!
    set PreEngineDefines=/DAPPLICATION_UNITY=\"!ApplicationUnityInclude!\"
)

if not "%ApplicationSettings%" == "" (
    set ApplicationSettingsInclude=%ApplicationDirectory%\%ApplicationSettings%
    echo "ApplicationSettings:      " !ApplicationSettingsInclude!
    set PostEngineDefines=/DAPPLICATION_SETTINGS=\"!ApplicationSettingsInclude!\"
)

if not defined DebugFlags (
    echo -- release mode
    set DebugFlags=-DNDEBUG
) else (
    echo -- debug mode
)

pushd %BinDirectory%

if defined AdressSanitizer (
    echo -- using clang-cl
    clang-cl  %CompilerFlags%                                                                                                   ^
        %OptimizationFlags%                                                                                                     ^
        %DebugFlags%                                                                                                            ^
        %AdressSanitizer%                                                                                                       ^
        %EngineDefines%                                                                                                         ^
        %PreEngineDefines% %PostEngineDefines%                                                                                  ^
        %Include_SDL% %Include_gl3w% %Include_stb% %Include_cJSON% %Include_fast_obj% %Include_klib% %Include_ImGui%            ^
        %Include_Common%                                                                                                        ^
        %Source_Engine% %Source_gl3w%                                                                                           ^
        /link %Library_SDL% %Library_OpenGL% %Library_cJSON% %Library_fast_obj% %Library_ImGui% %Library_stb% %Library_win32%   ^
        /SUBSYSTEM:CONSOLE

) else (
    echo -- using cl
    cl  %CompilerFlags%                                                                                                         ^
        %OptimizationFlags%                                                                                                     ^
        %DebugFlags%                                                                                                            ^
        %EngineDefines%                                                                                                         ^
        %PreEngineDefines% %PostEngineDefines%                                                                                  ^
        %Include_SDL% %Include_gl3w% %Include_stb% %Include_cJSON% %Include_fast_obj% %Include_klib% %Include_ImGui%            ^
        %Include_Common%                                                                                                        ^
        %Source_Engine% %Source_gl3w%                                                                                           ^
        /link %Library_SDL% %Library_OpenGL% %Library_cJSON% %Library_fast_obj% %Library_ImGui% %Library_stb% %Library_win32%   ^
        /SUBSYSTEM:CONSOLE

)

popd

echo -------- Finished compiling source

endlocal

exit /B %ERRORLEVEL%
