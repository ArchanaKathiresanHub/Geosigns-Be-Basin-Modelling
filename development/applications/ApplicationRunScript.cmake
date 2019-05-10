#########################################################################
#                                                                       #
# Copyright (C) 2012-2015 Shell International Exploration & Production. #
# All rights reserved.                                                  #
#                                                                       #
# Developed under license for Shell by PDS BV.                          #
#                                                                       #
# Confidential and proprietary source code of Shell.                    #
# Do not distribute without written permission from Shell.              #
#                                                                       #
#########################################################################

# The following create_application_run_script macro creates a script for running 
# the associated target. The input 'targetName' is supposed to be in the form
# <TARGET>.exe, the script will strip the extension and generate a bash file <TARGET>
# which sources the environment variables script <TARGET>Env.sh generated by the macro
# 'application_env_var', set the LD_LIBRARY_PATH to the current folder containing the scipt
# and run the application <TARGET>.exe
macro( create_application_run_script targetName )
if(UNIX)
    # Run script filename
    set( scriptName "${CMAKE_CURRENT_BINARY_DIR}/${targetName}" )
    # Environment variables script filename
    set( scriptVarName "${CMAKE_CURRENT_BINARY_DIR}/${targetName}Env.sh" )
    # Create file overwriting any existing content
    file( WRITE ${scriptName}
"#!/bin/bash -e
# Source this file to load the development environment into your shell
# This file has been automatically generated -- Do not edit!

# Script directory
DIR=\"$( cd \"$( dirname \"\${BASH_SOURCE\[0\]}\" )\" && pwd )\"
if [ -e $DIR/../misc ]; then
   MISCDIR=$DIR/../misc
else
   MISCDIR=$DIR/../../misc
fi

# Sourcing environment variables script (if it exists)
[[ -f \"$DIR/${targetName}Env.sh\" ]] && source $DIR/${targetName}Env.sh

# Setting runtime libraries directory
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$DIR

if [ -e /glb/data/hpcrnd/easybuild/PRODUCTION/software/rhel/6/GCCcore/4.9.3/lib64/libstdc++.so ]; then
   export LD_PRELOAD=/glb/data/hpcrnd/easybuild/PRODUCTION/software/rhel/6/GCCcore/4.9.3/lib64/libstdc++.so
fi
if [ -e /glb/data/hpcrnd/easybuild/public/etc/profile.d/shell-envmodules.sh ]; then
   . /glb/data/hpcrnd/easybuild/public/etc/profile.d/shell-envmodules.sh
fi
   
# In Shell network FLEXLM licens is requred and located in this file
if [ -z \"$SIEPRTS_LICENSE_FILE\" ] && [ -e /apps/sss/etc/local/flexlmlicenses ]; then
  export SIEPRTS_LICENSE_FILE=`cat /apps/sss/etc/local/flexlmlicenses | grep SIEPRTS_LICENSE_FILE | cut -d ' ' -f 2`
fi

module purge
module load intel/${INTEL_CXX_VERSION}
")

if( NOT CMAKE_BUILD_TYPE STREQUAL "Release" )
    file( APPEND ${scriptName}
"# Special module to debug with intel compiler 2017 in Shell
if [ -e /glb/data/hpcrnd/easybuild/public/etc/profile.d/shell-envmodules.sh ]; then
   module load GDB/7.8.2-GCC-4.9.2
fi
")
endif()

    file( APPEND ${scriptName}
"
# Running application forwarding all the script inputs
$DIR/${targetName}.exe \"$@\"
")

    # Applying permission to run script
    add_custom_command(TARGET ${targetName} POST_BUILD COMMAND chmod 755 ${scriptName} )
    # Applying permission to environment variables (if it exists, otherwise errors will be ignored)
    add_custom_command(TARGET ${targetName} POST_BUILD COMMAND chmod -f 755 ${scriptVarName} || : )
    # Installing permission to run script
    install( FILES "${scriptName}" 
             DESTINATION bin
             PERMISSIONS
               OWNER_READ OWNER_EXECUTE
               GROUP_READ GROUP_EXECUTE
               WORLD_READ WORLD_EXECUTE )
    # Installing permission to environment variables (if it exists)
    install( FILES "${scriptVarName}" 
             DESTINATION bin
             PERMISSIONS
               OWNER_READ OWNER_EXECUTE
               GROUP_READ GROUP_EXECUTE
               WORLD_READ WORLD_EXECUTE
             OPTIONAL )
endif(UNIX)
endmacro()

# The following application_env_var macro allows to create a script for setting an arbitrary
# number of environment variables for the current target. The macro takes in input a TARGET and
# a variable number of pairs of variable names and values grouped in the VARS input
# Example usage:
#   application_env_var( TARGET fastcauldron.exe VARS "VERSION" "2017.05" "ENV_VAR_EXAMPLE" "content" )
#   The generated script will contain the following
#   > export VERSION=2017.05
#   > export ENV_VAR_EXAMPLE=content
macro( APPLICATION_ENV_VAR )
if(UNIX)
    set( oneValueArgs TARGET )
    set( multiValueArgs VARS CUSTOM_COMMAND )
    cmake_parse_arguments(APPLICATION_ENV_VAR "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )
    set( scriptName "${CMAKE_CURRENT_BINARY_DIR}/${APPLICATION_ENV_VAR_TARGET}Env.sh" )
    if( EXISTS ${scriptName} )
        file( REMOVE ${scriptName} )
    endif()
    file( WRITE ${scriptName}
"#!/bin/bash -e
# Environment variables for ${APPLICATION_ENV_VAR_TARGET}

# Script directory
DIR=\"$( cd \"$( dirname \"\${BASH_SOURCE\[0\]}\" )\" && pwd )\"

" )
    set( index 0 )
    foreach(v ${APPLICATION_ENV_VAR_VARS})
        math( EXPR isVarName "${index}%2" )
        if( ${isVarName} EQUAL 0 )
            file( APPEND ${scriptName} "export ${v}=" )
        else()
            file( APPEND ${scriptName} "${v}
" )
        endif()
        math( EXPR index "${index}+1" )
    endforeach()

    foreach(v ${APPLICATION_ENV_VAR_CUSTOM_COMMAND})
       file( APPEND ${scriptName} "${v}
")
    endforeach()

endif(UNIX)
endmacro()

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
