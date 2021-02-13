@echo off
cls
setlocal
set P=%cd%\..

REM call make_externals.bat
call make_data.bat bin data

echo -------- Compiling source

set PathCompiled=%P%\win_x64\bin\obj\
if not exist %PathCompiled% mkdir %PathCompiled%

set PathPDB=%P%\win_x64\bin\

set ExecutableName=Application

set SourceApplication=%P%\source\engine\unity_main.cpp

set Include_gl3w=/I %P%\externals\gl3w\include\
set Source_gl3w=%P%\externals\gl3w\src\gl3w.c
set Library_OpenGL=opengl32.lib

set Include_stb=/I %P%\externals\stb
set Library_stb=%P%\externals\lib\stb.lib

set Include_SDL=/I %P%\externals\SDL2-2.0.10\include
set Library_SDL=/LIBPATH:%P%\externals\SDL2-2.0.10\x64 SDL2.lib SDL2main.lib

set Include_cJSON=/I %P%\externals\cJSON\
set Library_cJSON=%P%\externals\lib\cjson.lib

set Include_fast_obj=/I %P%\externals\fast_obj\
set Library_fast_obj=%P%\externals\lib\fast_obj.lib

set Include_klib=/I %P%\externals\klib\

set Library_win32=user32.lib gdi32.lib

set CompilerFlags=/nologo /Fe%Executablename% /FC /std:c++17 /cgthreads4 /Fo%PathCompiled%
set OptimizationFlags=/MT /GR- /EHsc /EHa- /Oi /O2
set DebugFlags=/Od /Zi /Fd%PathPDB%
set AdressSanitizer=-fsanitize=address

set Defines=/DLIB_STB /DLIB_CJSON /DLIB_FAST_OBJ /DPLATFORM_LAYER_SDL /DRENDERER_OPENGL3 /DDEVELOPPER_MODE
if not defined DebugFlags (
    echo -- release mode
    set DebugFlags=-DNDEBUG
) else (
    echo -- debug mode
)

pushd %P%\win_x64\bin

if defined AdressSanitizer (
    echo -- using clang-cl
    clang-cl  %CompilerFlags%                                                                                   ^
        %OptimizationFlags%                                                                                     ^
        %DebugFlags%                                                                                            ^
        %AdressSanitizer%                                                                                       ^
        %Defines%                                                                                               ^
        %Include_SDL% %Include_gl3w% %Include_stb% %Include_cJSON% %Include_fast_obj% %Include_klib%            ^
        %SourceApplication% %Source_gl3w%                                                                       ^
        /link %Library_SDL% %Library_OpenGL% %Library_stb% %Library_cJSON% %Library_fast_obj% %Library_win32%   ^
        /SUBSYSTEM:CONSOLE

) else (
    echo -- using cl
    cl  %CompilerFlags%                                                                                         ^
        %OptimizationFlags%                                                                                     ^
        %DebugFlags%                                                                                            ^
        %Defines%                                                                                               ^
        %Include_SDL% %Include_gl3w% %Include_stb% %Include_cJSON% %Include_fast_obj% %Include_klib%            ^
        %SourceApplication% %Source_gl3w%                                                                       ^
        /link %Library_SDL% %Library_OpenGL% %Library_stb% %Library_cJSON% %Library_fast_obj% %Library_win32%   ^
        /SUBSYSTEM:CONSOLE

)

popd

echo.
echo -------- Finished compiling source

endlocal
