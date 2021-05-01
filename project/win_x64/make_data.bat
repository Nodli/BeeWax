@echo off

set P=%cd%\..
set BinDir=%1
set DataDir=%2

echo -------- Mirroring data

mkdir %P%\win_x64\%BinDir%
xcopy %P%\data %P%\win_x64\%BinDir%\%DataDir% /E /Q /I /D /Y
xcopy %P%\externals\SDL2-2.0.10\x64\SDL2.dll %P%\win_x64\%BinDir% /Q /I /D /Y

echo.
echo -------- Finished mirroring data
