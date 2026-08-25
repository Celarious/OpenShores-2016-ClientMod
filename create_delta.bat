@echo off
if "%~2"=="" (
    echo Please drag two files onto this batch file.
    pause
    exit /b
)

echo.
echo File 1: "%~1"
echo File 2: "%~2"
echo.

set /p "choice=Is %~n1 the OLD file? (Y/N): "
set /p "outname=Enter output filename: "

if /I "%choice%"=="Y" (
    set "old=%~1"
    set "new=%~2"
) else (
    set "old=%~2"
    set "new=%~1"
)

echo.
echo Old file: "%old%"
echo New file: "%new%"
echo.

xdelta3 -f -e -s "%old%" "%new%" "%outname%.xdelta"

if errorlevel 1 (
    echo.
    echo ERROR: xdelta failed.
) else (
    echo.
    echo %outname%.xdelta created successfully.
)

pause