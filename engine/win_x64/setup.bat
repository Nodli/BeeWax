@echo off

set PathVS2017="C:\Program Files (x86)\Microsoft Visual Studio\2017\Professional\VC\Auxiliary\Build\vcvarsall.bat"
set PathVS2019="C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat"
set PathCLang="C:\Program Files\LLVM\bin"
set PathEmscriptem="C:\Users\rambu\Desktop\emsdk"
set PathVim="C:\Program Files (x86)\Vim\vim82"

if exist %PathVS2017% call %PathVS2017% x64
if exist %PathVS2019% call %PathVS2019% x64
if exist %PathCLang% set PATH=%PATH%;%PathCLang%;%PathVim%
if exist %PathEmscriptem% call %PathEmscriptem%\emsdk_env.bat
