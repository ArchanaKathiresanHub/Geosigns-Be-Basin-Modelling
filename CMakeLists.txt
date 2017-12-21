#########################################################################
#                                                                       #
# Copyright (C) 2012-2014 Shell International Exploration & Production. #
# All rights reserved.                                                  #
#                                                                       #
# Developed under license for Shell by PDS BV.                          #
#                                                                       #
# Confidential and proprietary source code of Shell.                    #
# Do not distribute without written permission from Shell.              #
#                                                                       #
#########################################################################

#
# Note: This CMake project serves the sole purpose of rolling a source
#       distribution. To build the project, use the CMake project in the
#       'development' subdirectory.
#

cmake_minimum_required(VERSION 2.8)
project( BasinModeling )

# Define a macro to get the current date
macro(today RESULT)
   if (UNIX)
      execute_process(COMMAND "date" "+%Y-%m-%d" OUTPUT_VARIABLE ${RESULT})
      string(REGEX REPLACE "(....)-(..)-(..).*" "\\1\\2\\3" ${RESULT} ${${RESULT}})
   elseif(WIN32)
        execute_process(COMMAND "cmd" " /C date /T" OUTPUT_VARIABLE ${RESULT})
        string(REGEX REPLACE "(..)/(..)/..(..).*" "\\1\\2\\3" ${RESULT} ${${RESULT}})
   else (UNIX)
      message(WARNING "date not implemented.")
      set(${RESULT} "000000")
   endif (UNIX)
endmacro(today)

# Define a macro to get the current Changeset ID of the source in the source
# repository
macro(get_changeset_id RESULT)
   if (UNIX)
      execute_process(COMMAND "git" "describe" 
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            OUTPUT_VARIABLE ${RESULT}
            OUTPUT_STRIP_TRAILING_WHITESPACE
            RESULT_VARIABLE gitReturnedError
       )
      if (gitReturnedError)
        message(WARNING "Cannot retrieve version info, because local directory is not part of a git repository.")
      endif()
   else(UNIX)
      message(WARNING "Cannot retrieve version info, because I don't know how to get version info on non-Unix systems.")
      set(${RESULT} "000000")
   endif(UNIX)
endmacro(get_changeset_id)

   
# Let's simplify and only make a .tar.gz file
set( CPACK_SOURCE_GENERATOR "TGZ")

# There are some files we don't want to distribute, because the source package
# will get very big.
set( CPACK_SOURCE_IGNORE_FILES 
      # general ignore pattern
      "\\\\.swp$" "~$" "/\\\\.git/" "/\\\\.tf/" "\\\\.tpattributes$" 

      # Geocase is no part of Cauldron anymore
      "/geocase/BB/"
      "/geocase/misc/GospelScriptFiles/"

      # These versions of 3rd party libs are not being used
      "/3rdparty/sources/eigen-3.2.2.tar.gz"
      "/3rdparty/sources/petsc-3.4.5.tar.gz"
      
      # There is no need to redistribute Geocosm stuff
      "/3rdparty/sources/geocosm.tar.gz"
      "/3rdparty/sources/tslibredist_2014-09-03_7-4-2-4680.zip"

      # Other non-relevant stuff
      "/inversion/"
      "/tests/"
   )

# Set the version of the package
if (NOT CPACK_PACKAGE_VERSION)
   today(currentDate)
   get_changeset_id(changeset)
   set( CPACK_PACKAGE_VERSION "${changeset}-packed_on-${currentDate}")
endif()
message(STATUS "Package version = ${CPACK_PACKAGE_VERSION}")

# Include CPack
include(CPack)

# Print a note that this file won't build the project
message("NOTE: The purpose of this CMake file is to make a source "
      "distribution of the Cauldron. Use the CMake file in the 'development' "
      "subdir to really build it.")
