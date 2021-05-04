@echo off
setlocal

echo -------- Generating external libraries

pushd %~dp0
set RootDirectory=%cd%\..\..
set ExternalsDirectory=%RootDirectory%\externals
popd

set LibraryPath=%ExternalsDirectory%\lib
if not exist %LibraryPath% mkdir %LibraryPath%

set Source_stb=%ExternalsDirectory%\stb\stb_source.cpp
set Library_stb=stb.lib
set Compiled_stb=stb.obj

set Source_cJSON=%ExternalsDirectory%\cJSON\cJSON.c
set Library_cJSON=cjson.lib
set Compiled_cJSON=cjson.obj

set Source_fast_obj=%ExternalsDirectory%\fast_obj\fast_obj_source.cpp
set Library_fast_obj=fast_obj.lib
set Compiled_fast_obj=fast_obj.obj

set Source_ImGui=%ExternalsDirectory%\imgui\imgui_source.cpp
set Library_ImGui=imgui.lib
set Compiled_ImGui=imgui.obj
set External_ImGui=/I %ExternalsDirectory%\stb /I %ExternalsDirectory%\gl3w\include /I %ExternalsDirectory%\SDL2-2.0.10\include

REM set DebugFlags=/Od /Z7 /DDEBUG
if not defined DebugFlags (
    echo -- release mode
    set DebugFlags=-DNDEBUG
) else (
    echo -- debug mode
)

pushd %LibraryPath%

echo -------- stb

@echo on
cl /nologo /c /FC /EHsc /O2 %DebugFlags% /Fo%LibraryPath%/%Compiled_stb% %Source_stb%
lib /nologo %LibraryPath%/%Compiled_stb% /out:%LibraryPath%\%Library_stb%
@echo off

echo -------- cJSON

@echo on
cl /nologo /c /FC /EHsc /O2 %DebugFlags% /Fo%LibraryPath%/%Compiled_cJSON% %Source_cJSON%
lib /nologo %LibraryPath%/%Compiled_cJSON% /out:%LibraryPath%\%Library_cJSON%
@echo off

echo -------- fast_obj

@echo on
cl /nologo /c /FC /EHsc /O2 %DebugFlags% /Fo%LibraryPath%/%Compiled_fast_obj% %Source_fast_obj%
lib /nologo %LibraryPath%/%Compiled_fast_obj% /out:%LibraryPath%\%Library_fast_obj%
@echo off

echo -------- ImGui

@echo on
cl /nologo /c /FC /EHsc /O2 %DebugFlags% /Fo%LibraryPath%/%Compiled_ImGui% %Source_ImGui% %External_ImGui%
lib /nologo %LibraryPath%/%Compiled_ImGui% /out:%LibraryPath%\%Library_ImGui%
@echo off

popd

echo -------- Finished generating external libraries

endlocal
