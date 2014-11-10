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

mark_as_advanced( 
      BM_C_SHELL_ENVIRONMENT_SCRIPT_FILE
      BM_BOURNE_SHELL_ENVIRONMENT_SCRIPT_FILE
      BM_WINCMD_SHELL_ENVIRONMENT_SCRIPT_FILE
)

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
   set( BM_ENV_${varName} "${varValue}" CACHE STRING "Environment variable" )
   set( value "${BM_ENV_${varName}}" )
   file(APPEND "${CMAKE_BINARY_DIR}/${BM_C_SHELL_ENVIRONMENT_SCRIPT_FILE}" "setenv ${varName} \"${value}\"\n")
   file(APPEND "${CMAKE_BINARY_DIR}/${BM_BOURNE_SHELL_ENVIRONMENT_SCRIPT_FILE}" "export ${varName}=\"${value}\"\n")
   file(APPEND "${CMAKE_BINARY_DIR}/${BM_WINCMD_SHELL_ENVIRONMENT_SCRIPT_FILE}" "set ${varName}=${value}\n")
endmacro(add_environment_variable)

# - Write a shell script that can wraps an executable
# Some executables that are necessary to build this project, need a special
# environment. The following functions create a wrapper for such an executable
# by first setting-up this special environment and then calling the executable.
#
# add_environment_source_script_to_wrapper
#        Adds a line to the script that sources a specified script.
# 
#          add_environment_source_script( <wrapper-name> <script-name> )
#
#
# finish_wrapper
#        Writes the wrapper and makes it executable
#
#           finish_wrapper( <wrapper-name> <executable> <variable-name> )
#        
#        Appends the a call to <executable> and forwards all positional
#        paraemeters. The file name of the resulting wrapper is written
#        to variable <variable-name>.

macro(add_environment_source_script_to_wrapper name scriptName)
   file(APPEND "${CMAKE_BINARY_DIR}/aux/${name}_wrap_environment.sh" "source ${scriptName}\n")
endmacro(add_environment_source_script_to_wrapper)

macro(finish_wrapper name executable wrapper)
   # First read the set of scripts to be sourced
   file(READ "${CMAKE_BINARY_DIR}/aux/${name}_wrap_environment.sh" compilerEnvironment)

   # Now write the wrapper
   file(WRITE "${CMAKE_BINARY_DIR}/aux/${name}_wrap.sh" 
"#!/bin/bash
# Compiler wrapper for the ${compiler} compiler
# This file has been automatically generated -- Do not edit!

# Mark all variables list below to be exported to the environment
${compilerEnvironment}

# Run the tool
${executable}
"
       )

   if (EXISTS "${CMAKE_BINARY_DIR}/${name}_wrap.sh")
     file(REMOVE "${CMAKE_BINARY_DIR}/${name}_wrap.sh")
   endif()
   file(COPY "${CMAKE_BINARY_DIR}/aux/${name}_wrap.sh"
        DESTINATION "${CMAKE_BINARY_DIR}"
        FILE_PERMISSIONS
           OWNER_READ OWNER_WRITE OWNER_EXECUTE
           GROUP_READ GROUP_WRITE GROUP_EXECUTE
           WORLD_READ WORLD_EXECUTE
        )

   set(${wrapper} "${CMAKE_BINARY_DIR}/${name}_wrap.sh")
endmacro(finish_wrapper)


