@echo off

echo -------- Generating external libraries

set externals=%cd%\..\externals

set LibraryPath=%externals%\lib
if not exist %LibraryPath% mkdir %LibraryPath%

set Source_stb=%externals%\stb\stb_implementation.cpp
set Library_stb=stb.lib
set Compiled_stb=stb.obj

set Source_cJSON=%externals%\cJSON\cJSON.c
set Library_cJSON=cjson.lib
set Compiled_cJSON=cjson.obj

set Source_fast_obj=%externals%\fast_obj\fast_obj_implementation.cpp
set Library_fast_obj=fast_obj.lib
set Compiled_fast_obj=fast_obj.obj

pushd %LibraryPath%

echo -------- stb

@echo on
cl /nologo /c /FC /EHsc /O2 /Fo%LibraryPath%/%Compiled_stb% %Source_stb%
lib /nologo %LibraryPath%/%Compiled_stb% /out:%LibraryPath%\%Library_stb%
@echo off

echo.
echo -------- cJSON

@echo on
cl /nologo /c /FC /EHsc /O2 /Fo%LibraryPath%/%Compiled_cJSON% %Source_cJSON%
lib /nologo %LibraryPath%/%Compiled_cJSON% /out:%LibraryPath%\%Library_cJSON%
@echo off

echo.
echo -------- fast_obj

@echo on
cl /nologo /c /FC /EHsc /O2 /Fo%LibraryPath%/%Compiled_fast_obj% %Source_fast_obj%
lib /nologo %LibraryPath%/%Compiled_fast_obj% /out:%LibraryPath%\%Library_fast_obj%
@echo off

echo.
echo -------- Finished generating external libraries

popd
