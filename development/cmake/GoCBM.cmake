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

#
# Writes environment set-up script to run Cauldron applications.
#

add_environment_variable(IBS_LIMITEDBUILDANDINSTALL true)
   
# Set the field width in the tables that TableIO outputs
add_environment_variable(IBS_FIELDWIDTH 24)

# Set environment variables to various configuration files that are used by the applications
add_environment_variable( EOSPACKDIR "${PROJECT_SOURCE_DIR}/geocase/misc/eospack")
add_environment_variable( GENEXDIR "${PROJECT_SOURCE_DIR}/geocase/misc/genex40  ")
add_environment_variable( GENEX5DIR "${PROJECT_SOURCE_DIR}/geocase/misc/genex50 ")
add_environment_variable( GENEX6DIR "${PROJECT_SOURCE_DIR}/geocase/misc/genex60")
add_environment_variable( OTGCDIR "${PROJECT_SOURCE_DIR}/geocase/misc/OTGC ")
add_environment_variable( CTCDIR "${PROJECT_SOURCE_DIR}/geocase/misc")

# Setting the PATH variable to /apps/sssdev
add_environment_path( "/apps/sssdev/share" )

# Add current CMake executable to the path
get_filename_component(CMAKE_COMMAND_PATH "${CMAKE_COMMAND}" PATH CACHE)
add_environment_path( "${CMAKE_COMMAND_PATH}" )

# Add license server
add_environment_variable( SIEPRTS_LICENSE_FILE
      "3000@ams1-s-07489.europe.shell.com:3000@houic-s-9320.americas.shell.com:3000@cbj-s-8447.asia-pac.shell.com"
)
#NOTE: This variables can be reverse engineered by running the a IBS application
#      with CSCE_DEBUG=1 set
# For example: CSCE_DEBUG=1 ksh -x /apps/sssdev/share/fastcauldron -v2013.0504
# Shows all scripts that are executed to run fastcauldron


