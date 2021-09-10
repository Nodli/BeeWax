@echo off

cls

echo -------- Compilation

call make
set ReturnCode=%ERRORLEVEL%

if %ReturnCode% == 0 (
    echo -------- Execution
    call run
)

echo -------- Finished
