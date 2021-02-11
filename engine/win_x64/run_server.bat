@echo off

pushd %cd%\ems
call ipconfig
call python -m http.server
popd
