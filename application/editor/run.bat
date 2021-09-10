@echo off

pushd %~dp0\bin

Application.exe

popd

exit /B %ERRORLEVEL%
