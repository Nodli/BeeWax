@echo off
cls

set P=%cd%\..

REM call make_externals.bat
call make_spirv.bat
call make_data.bat bin data

echo -------- Compiling source

set PathCompiled=%P%\win_x64\bin\obj\
if not exist %PathCompiled% mkdir %PathCompiled%

set PathPDB=%P%\win_x64\bin\

set ExecutableName=Application

set SourceApplication=%P%\source\engine\unity_main.cpp

set Include_vulkan=/I %P%\externals\vulkan\
set Library_vulkan=/LIBPATH:%P%\externals\vulkan\x64 vulkan-1.lib VkLayer_utils.lib

set Include_stb=/I %P%\externals\stb
set Library_stb=%P%\externals\lib\stb.lib

set Include_SDL=/I %P%\externals\SDL2-2.0.10\include
set Library_SDL=/LIBPATH:%P%\externals\SDL2-2.0.10\x64 SDL2.lib SDL2main.lib

set Include_cJSON=/I %P%\externals\cJSON\
set Library_cJSON=%P%\externals\lib\cjson.lib

set Include_klib=/I %P%\externals\klib\

set Library_win32=user32.lib gdi32.lib

set CompilerFlags=/nologo /Fe%Executablename% /FC /EHsc /std:c++17 /O0 /cgthreads4 /Fo%PathCompiled%
set DebugFlags=/Zi /Fd%PathPDB%
REM set DebugFlags=

set Defines=/DPLATFORM_LAYER_SDL /DRENDERER_VULKAN /DLIB_STB /DLIB_CJSON /DDEVELOPPER_MODE

pushd %P%\win_x64\bin

cl  %CompilerFlags%                                                                     ^
	%DebugFlags%                                                                        ^
	%Defines%                                                                           ^
	%Include_SDL% %Include_vulkan% %Include_stb% %Include_cJSON% %Include_klib%         ^
	%SourceApplication%                                                                 ^
	/link %Library_SDL% %Library_vulkan% %Library_stb% %Library_cJSON% %Library_win32%  ^
	/SUBSYSTEM:CONSOLE

echo.
echo -------- Finished compiling source

popd
