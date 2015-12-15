@ECHO OFF
SETLOCAL

REM #########################################################################
REM #                                                                       #
REM # Copyright (C) 2015-2016 Shell International Exploration & Production. #
REM # All rights reserved.                                                  #
REM #                                                                       #
REM # Developed under license for Shell by PDS BV.                          #
REM #                                                                       #
REM # Confidential and proprietary source code of Shell.                    #
REM # Do not distribute without written permission from Shell.              #
REM #                                                                       #
REM #########################################################################

@ECHO ON
REM **********************************************************
REM ***************Creating build directory*******************
REM **********************************************************
@ECHO OFF
REM Build directory and Install directory must be specified as a command line parameter:
REM Windows-CreateBuildDirectory.bat <BuildDirectory> <InstallDirectory>

REM -----------------------------------
REM ---Set directories-----------------
REM -----------------------------------
Set BuildDirectory=%1
Set InstallDirectory=%2

REM -----------------------------------
REM ---Remove and clean directories----
REM -----------------------------------
If exist %BuildDirectory% (
   Echo Cleaning old build directory "%BuildDirectory%"
   Rd /S /Q %BuildDirectory%
   Echo Old build directory "%BuildDirectory%" cleaned
)
If exist %InstallDirectory% (
   Echo Cleaning old install directory "%InstallDirectory%"
   Rd /S /Q %InstallDirectory%
   Echo Old install directory "%InstallDirectory%" cleaned
)

REM -----------------------------------
REM ---Create directories--------------
REM -----------------------------------
Echo Creating new build directory "%BuildDirectory%"
Mkdir %BuildDirectory%
Echo New build directory "%BuildDirectory%" created
Echo Creating new install directory "%InstallDirectory%"
Mkdir %InstallDirectory%
Echo New install directory "%InstallDirectory%" created

ENDLOCAL