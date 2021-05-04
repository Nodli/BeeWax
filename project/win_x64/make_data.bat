@echo off
setlocal

set DestinationDirectory=%1
set SourceDirectory=%2

echo -------- Mirroring data

echo "Source:       " %SourceDirectory%
echo "Destination:  " %DestinationDirectory%

mkdir %DestinationDirectory%
if not "%SourceDirectory%" == "" (
    xcopy %SourceDirectory% %DestinationDirectory% /E /Q /I /D /Y
) else (
    echo "WARNING: Source folder does not exist"
)

echo -------- Finished mirroring data
