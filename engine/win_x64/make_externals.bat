@echo off
cls
echo -------- Generating static libraries

echo STB...

set P=%cd%

set SourcePath=%P%\..\externals\stb
set FolderPath=%P%\..\externals\stb\static_library\windows_x64

if not exist %FolderPath% mkdir %FolderPath%
pushd %FolderPath%

set LibraryName=stb.lib
set ObjectName=stb.obj
@echo on

cl /nologo /c /EHsc /Fo%FolderPath%/%ObjectName% %SourcePath%\stb_libraries.cpp
lib /nologo %FolderPath%/%ObjectName% /out:%FolderPath%\%LibraryName%

@echo off
popd
echo Done
@echo on
