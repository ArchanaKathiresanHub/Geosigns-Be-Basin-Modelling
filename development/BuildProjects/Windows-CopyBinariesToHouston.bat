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
REM ***************Copying binairies to Houston***************
REM **********************************************************
@ECHO OFF
REM Deploy and Zip directories must be specified as a command line parameter:
REM Windows-CopyBinariesToHouston.bat <DeployDirectory> <ZipDirectory>

REM -----------------------------------
REM ---Linux host information----------
REM -----------------------------------
Set AgentHost=houcy1-n-sp030b01.americas.shell.com
Set AgentAccount=s_bpac00
Set AgentKey=C:\opt\putty\s_bpac00.tfs_build_server.ppk
Set DeployDirectory=%1

REM -----------------------------------
REM ---Binaries zip file information---
REM -----------------------------------
REM The path should be in scp format:
REM    scp accepts path in unix format, and cygwin has all windows drives mounted under /cygdrive/ root folder without ':' in drive name,
REM    like this: /cygdrive/f/cauldron. That is why we need to replace \ with / and remove :
REM    spaces in the folder paths require to put path in "" whichs not allows to expand wildcards - remove spaces from folder name 
REM    "Basin Modeling" - it is linked on build server to "BasingModeling" for windows builds.
Set ZipDirectory=%2
REM 1) Switching '\' to '/'
Set zip_dir_scp_format=%ZipDirectory:\=/%
REM 2) Switching ':' to ''
Set zip_dir_scp_format=%zip_dir_scp_format::=%
REM 3) Switching ' ' to ''
Set zip_dir_scp_format=%zip_dir_scp_format: =%
REM 4) Use cygdrive
Set zip_dir_scp_format=/cygdrive/%zip_dir_scp_format%

REM -----------------------------------
REM ---Standard utilities information--
REM -----------------------------------
Set cygwin_bin_dir=C:\opt\cygwin\bin\
Set ssh_exe=%cygwin_bin_dir%ssh.exe
Set scp_exe=%cygwin_bin_dir%scp.exe
Set scp_command=echo y ^| %scp_exe% -q

REM -----------------------------------
REM ---Check scp command parameters----
REM -----------------------------------
REM 1) Check ssh AgentKey
If not exist %AgentKey% (
	echo ERROR: Ssh agent key not found at %AgentKey%
	exit /b 111
)
REM 2) Check cygwin directory
If not exist %cygwin_bin_dir% (
	echo ERROR: Cygwin not found at %cygwin_bin_dir%
	exit /b 222
)
REM 3) Check for zip file with binaries
If not exist %ZipDirectory% (
	echo ERROR: Directory for zip file deployement not found at %ZipDirectory%
	exit /b 333
)

REM -----------------------------------
REM ----Copying to Houston-------------
REM -----------------------------------
REM 1) Clean Houston directory
Echo Cleaning old and creating new deployement directory "%DeployDirectory%"
%ssh_exe% %AgentAccount%@%AgentHost% "rm -Rf %DeployDirectory%; mkdir -p %DeployDirectory%"
Echo Old deployement directory "%DeployDirectory%" cleaned and created
REM 2) Copy zip file
Echo Copying zip file in "%zip_dir_scp_format%" to "%AgentAccount%@%AgentHost%:%DeployDirectory%"
%scp_command% -r %zip_dir_scp_format%/*.zip "%AgentAccount%@%AgentHost%:%DeployDirectory%"
Echo Zip file in "%zip_dir_scp_format%" copied to "%AgentAccount%@%AgentHost%:%DeployDirectory%"

ENDLOCAL