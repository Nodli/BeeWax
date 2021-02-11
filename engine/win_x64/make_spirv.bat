@echo off

set glslangProgram=%cd%\..\externals\vulkan\x64\glslangValidator.exe
set shaderFolder=%cd%\..\data\shader\
set shaderExtensions=*.vert *.frag
set spirvExtension=.spirv

echo -------- Generating shaders

for /f "delims=" %%a in ('dir /s /b %shaderExtensions%') do call :CompileShader %%a
goto EndCompileShader

:CompileShader
set NameExtension=%~nx1
set spirvFile=%NameExtension%%spirvExtension%
echo -- shader
echo in:  %NameExtension%
echo out: %spirvFile%
%glslangProgram% -V -g -o %shaderFolder%%spirvFile% %shaderFolder%%NameExtension%
goto :eof

:EndCompileShader

echo.
echo -------- Finished generating shaders
