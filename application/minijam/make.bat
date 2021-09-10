@echo off
setlocal

pushd %~dp0

..\..\project\win_x64\make.bat source\unity.cpp source\unity_settings.cpp

popd

exit /B %ERRORLEVEL%
