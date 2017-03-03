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
REM ***************Creating unit test directory***************
REM **********************************************************
@ECHO OFF
REM Unit test directory must be specified as a command line parameter:
REM Linux-CreateUnitTestDirectory.bat <UnitTestDirectory>

REM -----------------------------------
REM ---Set directories-----------------
REM -----------------------------------
Set UnitTestDirectory=%1

REM -----------------------------------
REM ---Remove and clean directory------
REM -----------------------------------
If exist %UnitTestDirectory% (
   Echo Cleaning old unit test directory "%UnitTestDirectory%"
   Rd /S /Q %UnitTestDirectory%
   Echo Old unit test directory "%UnitTestDirectory%" cleaned
)

REM -----------------------------------
REM ---Create directory----------------
REM -----------------------------------
Echo Creating new unit test directory "%UnitTestDirectory%"
Mkdir %UnitTestDirectory%
Echo New unit test directory "%UnitTestDirectory%" created

ENDLOCAL