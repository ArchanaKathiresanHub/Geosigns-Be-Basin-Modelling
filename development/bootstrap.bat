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
set HDF5_ROOT=%TOP_LEVEL_DIR%\..\3rdparty\hdf5.win64\hdf5

"C:\Program Files (x86)\CMake 3.2.1\bin\cmake.exe" -G"Visual Studio 12 Win64" %TOP_LEVEL_DIR% ^
	-DCMAKE_INSTALL_PREFIX=%CD%

"C:\Program Files (x86)\CMake 3.2.1\bin\cmake-gui.exe" -H%TOP_LEVEL_DIR% -B%CD%

:exit
pause
