#########################################################################
#                                                                       #
# Copyright (C) 2012-2013 Shell International Exploration & Production. #
# All rights reserved.                                                  #
#                                                                       #
# Developed under license for Shell by PDS BV.                          #
#                                                                       #
# Confidential and proprietary source code of Shell.                    #
# Do not distribute without written permission from Shell.              #
#                                                                       #
#########################################################################

# - Write a shell script that can be sourced to set-up a shell's environment.
# Many components, third party and internal, need specific environment
# settings to run. To write a script that can set the environment correctly
# there are several macros defined that will write a 'envsetup.sh',
# 'envsetup.csh', and a 'envsetup.bat' file:
#
# init_environment
#        Clears the environment set-up scripts.
#
#          init_environment()
#
# add_environment_source_script
#        Adds a line to the script that sources a specified script.
# 
#          add_environment_source_script( <CSHELL|BOURNE> <script-name> )
#
#        Depending on the shell that is used, C-shell or Bourne, specify
#        CSHELL or BOURNE, respectively, as the first argument. The second
#        argument should be the script file path.
#
# add_environment_path
#        Adds path to the PATH environment variable
#
#          add_environment_path( <path> )
#
#        Adds the <path> to the PATH environment variable
#
# add_environment_variable
#        Adds a variable to the environment and defines the same cmake variable
#
#          add_environment_variable( <name> <value> )
#
#        Sets the variable <name> to <value> in the environment
#

set(BM_C_SHELL_ENVIRONMENT_SCRIPT_FILE "envsetup.csh" CACHE FILEPATH "C-shell script to set-up environment")
set(BM_BOURNE_SHELL_ENVIRONMENT_SCRIPT_FILE "envsetup.sh" CACHE FILEPATH "Bourne-shell script to set-up environment")
set(BM_WINCMD_SHELL_ENVIRONMENT_SCRIPT_FILE "envsetup.bat" CACHE FILEPATH "Windows Cmd script to set-up environment")

macro(init_environment)
   file(WRITE "${CMAKE_BINARY_DIR}/${BM_C_SHELL_ENVIRONMENT_SCRIPT_FILE}" 
"#!/bin/csh
# Source this file to load the development environment into your C-shell
# This file has been automatically generated -- Do not edit!
")
   file(WRITE "${CMAKE_BINARY_DIR}/${BM_BOURNE_SHELL_ENVIRONMENT_SCRIPT_FILE}"
"#!/bin/sh
# Source this file to load the development environment into your Bourne shell
# This file has been automatically generated -- Do not edit!
")
   file(WRITE "${CMAKE_BINARY_DIR}/${BM_WINCMD_SHELL_ENVIRONMENT_SCRIPT_FILE}"
"REM Source this file to load the development environment into your shell
REM This file has been automatically generated -- Do not edit!
")
endmacro(init_environment)

macro(add_environment_source_script shellType scriptName)
   if (${shellType} STREQUAL "CSHELL")
      file(APPEND "${CMAKE_BINARY_DIR}/${BM_C_SHELL_ENVIRONMENT_SCRIPT_FILE}" "source ${scriptName}\n")
   elseif (${shellType} STREQUAL "BOURNE")
      file(APPEND "${CMAKE_BINARY_DIR}/${BM_BOURNE_SHELL_ENVIRONMENT_SCRIPT_FILE}" "source ${scriptName}\n")
   elseif(${shellType} STREQUAL "WINCMD")
      message(SEND_ERROR "Illegal command for Windows Cmd script")
   endif()
endmacro(add_environment_source_script)


macro(add_environment_path path)
   file(APPEND "${CMAKE_BINARY_DIR}/${BM_C_SHELL_ENVIRONMENT_SCRIPT_FILE}" "set path = ( ${path} \$path )\n")
   file(APPEND "${CMAKE_BINARY_DIR}/${BM_BOURNE_SHELL_ENVIRONMENT_SCRIPT_FILE}" "export PATH=${path}:\$PATH\n")
   file(APPEND "${CMAKE_BINARY_DIR}/${BM_WINCMD_SHELL_ENVIRONMENT_SCRIPT_FILE}" "set PATH=${path}:%PATH%\n")
endmacro(add_environment_path)

macro(add_environment_variable varName varValue)
   file(APPEND "${CMAKE_BINARY_DIR}/${BM_C_SHELL_ENVIRONMENT_SCRIPT_FILE}" "setenv ${varName} \"${varValue}\"\n")
   file(APPEND "${CMAKE_BINARY_DIR}/${BM_BOURNE_SHELL_ENVIRONMENT_SCRIPT_FILE}" "export ${varName}=\"${varValue}\"\n")
   file(APPEND "${CMAKE_BINARY_DIR}/${BM_WINCMD_SHELL_ENVIRONMENT_SCRIPT_FILE}" "set ${varName}=${varValue}\n")
   set( ${varName} ${varValue} CACHE STRING "Environment variable" )
endmacro(add_environment_variable)
