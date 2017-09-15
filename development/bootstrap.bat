@echo off

pushd %~dp0
set TOP_LEVEL_DIR=%CD%
popd

if NOT %TOP_LEVEL_DIR% == %CD% goto l1

echo -------------------------------------------------
echo Do not run the script inside the source directory
echo Please use bootstrap.vbs script
echo -------------------------------------------------
goto exit

:l1

c:\Apps\3rdparty\cmake-3.8.2\bin\cmake.exe -G"Visual Studio 14 Win64" %TOP_LEVEL_DIR% -DCMAKE_INSTALL_PREFIX=%CD%
c:\Apps\3rdparty\cmake-3.8.2\bin\cmake-gui.exe -H%TOP_LEVEL_DIR% -B%CD%

:exit
pause
