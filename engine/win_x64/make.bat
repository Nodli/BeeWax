@echo off

cls
echo "-------- Compilation"

set P=%cd%\..

mkdir %P%\win_x64\bin
xcopy %P%\data %P%\win_x64\bin\data /E /Q /I /D /Y
xcopy %P%\externals\SDL2-2.0.10\x64\SDL2.dll %P%\win_x64\bin /Q /I /D /Y

set PathOBJ=%P%\win_x64\bin\obj
if not exist %PathOBJ% mkdir %PathOBJ%

set ExecutableName=Application

set SourceApplication=%P%\source\engine\unity_main.cpp

set IncludeGL3W=/I %P%/externals/gl3w/include/
set SourceGL3W=%P%/externals/gl3w/src/gl3w.c

set IncludeSTB=/I %P%\externals\stb
set LinkSTB=%P%\externals\stb\static_library\win_x64\stb.lib

set IncludeSDL=/I %P%\externals\SDL2-2.0.10\include
set LinkSDL=/LIBPATH:%P%\externals\SDL2-2.0.10\x64 SDL2.lib SDL2main.lib

set LinkWindows=user32.lib gdi32.lib opengl32.lib

set CompilerFlags=/nologo /Fe%Executablename% /FC /EHsc /std:c++17 /O2 /cgthreads4 /Fo%PathOBJ%\
set DebugFlags=/Z7

pushd %P%\win_x64\bin

cl  %CompilerFlags%                                     ^
	/DLIB_STB /DPLATFORM_LAYER_SDL /DRENDERER_OPENGL3   ^
	%IncludeGL3W% %IncludeSTB% %IncludeSDL%             ^
	%SourceApplication% %SourceGL3W%                    ^
	/link %LinkSTB% %LinkSDL% %LinkWindows%             ^
	/SUBSYSTEM:CONSOLE

popd
