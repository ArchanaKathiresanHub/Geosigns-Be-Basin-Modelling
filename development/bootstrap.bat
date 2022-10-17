@echo off

pushd %~dp0
set TOP_LEVEL_DIR=%CD%

echo %CMAKE%
popd

if NOT %TOP_LEVEL_DIR% == %CD% goto l1

echo -------------------------------------------------
echo Do not run the script inside the source directory
echo Please use bootstrap.vbs script
echo -------------------------------------------------
goto exit


:l1

CMAKE -G "Visual Studio 17 2022" -A x64 %TOP_LEVEL_DIR% -DCMAKE_INSTALL_PREFIX=%CD% -H%TOP_LEVEL_DIR% -B%CD%

:exit
pause
