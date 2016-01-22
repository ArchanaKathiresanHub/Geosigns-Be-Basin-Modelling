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
# Try to find the LSF installation
#

set (LSF_LIBS)
set (LSF_INCLUDE_DIR)
set (LSF_HOME "LSF-NOTFOUND" CACHE PATH "Home dir for LSF binary files")
set (LSF_CAULDRON_PROJECT_NAME               "LSF-NOTFOUND" CACHE STRING "Project name for submitting jobs to LSF (for billing purpose)" )
set (LSF_CAULDRON_PROJECT_QUEUE              "LSF-NOTFOUND" CACHE STRING "LSF job queue name" )
set (LSF_CAULDRON_PROJECT_GROUP              "LSF-NOTFOUND" CACHE STRING "Project group" )
set (LSF_CAULDRON_PROJECT_SERVICE_CLASS_NAME "LSF-NOTFOUND" CACHE STRING "Project service class name" )

if(UNIX)
   find_path(LSF_INCLUDE_DIR lsf/lsbatch.h
      PATHS
         ${LSF_HOME}/../include
   )
   find_library(LSBATCH_LIB NAMES bat
      PATHS 
         ${LSF_HOME}/lib
   )
   find_library(LSF_LIB NAMES lsf
      PATHS 
         ${LSF_HOME}/lib
   )
   find_library(NSL_LIB nsl)
   find_library(MATH_LIB m)

   if (LSF_INCLUDE_DIR AND LSF_LIB AND LSBATCH_LIB)
      set(LSF_FOUND "Yes")
      mark_as_advanced(LSF_INCLUDE_DIR)
      mark_as_advanced(LSBATCH_LIB)
      mark_as_advanced(LSF_LIB)

      list(APPEND LSF_LIBS "${LSBATCH_LIB}" "${LSF_LIB}" "${MATH_LIB}" "${NSL_LIB}")
	  set (LSF_LICENSE_FILE_PATH "${LSF_HOME}/../license/English.txt" CACHE STRING "LSF licence file" )

   endif (LSF_INCLUDE_DIR AND LSF_LIB AND LSBATCH_LIB)
   
else (UNIX) # WIN32

   set (LSF_INCLUDE_DIR "${THIRD_PARTY_DIR}/LSF/include")
   set (LSF_LIB "${THIRD_PARTY_DIR}/LSF/lib/liblsf.lib")
   set (LSBATCH_LIB "${THIRD_PARTY_DIR}/LSF/lib/libbat.lib")
   
   mark_as_advanced(LSF_INCLUDE_DIR)
   mark_as_advanced(LSBATCH_LIB)
   mark_as_advanced(LSF_LIB)

   list(APPEND LSF_LIBS "${LSBATCH_LIB}" "${LSF_LIB}" "MsWSock.lib" "WS2_32.lib" "WSock32.lib" "adsiid.lib" "activeds.lib" "userenv.lib" "netapi32.lib" "mpr.lib")  
   set (LSF_ENVDIR "${LSF_HOME}/conf" )
   set (LSF_LICENSE_FILE_PATH "${THIRD_PARTY_DIR}/licenses/LSF-9.1.txt" CACHE STRING "LSF licence file" )
   set (LSF_FOUND "Yes")
   
endif (UNIX)

#Print status of LSF
#MESSAGE( STATUS "LSF_FOUND "       ${LSF_FOUND} )
#MESSAGE( STATUS "LSF_INCLUDE_DIR " ${LSF_INCLUDE_DIR} )
#MESSAGE( STATUS "LSF_LIB "         ${LSF_LIB} )
#MESSAGE( STATUS "LSF_LIBS "        ${LSF_LIBS} )

# Add licensen and ECCN information 
if (LSF_FOUND)

   set (LSF_ENVDIR ${LSF_HOME}/../../conf )
   add_definitions(-DWITH_LSF_SCHEDULER)

   include(cmake/AddPackage.cmake)

   add_external_package_info(
      CAPABILITY LSF
      NAME    "LSF"
      VENDOR  "IBM"
      VERSION "9.1.1"
      LICENSE_TYPE "Commercial"
      LICENSE_FILE "${LSF_LICENSE_FILE_PATH}"
      URL "http://www-03.ibm.com/systems/platformcomputing/products/lsf/index.html"
      DESCRIPTION "Load Sharing Facility (LSF) is a workload management platform, job scheduler, for distributed HPC environments"
      REQUIRED_AT  "Runtime"
      COUNTRY_OF_ORIGIN "USA"
      SHIPPED      "Yes"
      INCLUSION_TYPE "Static Link"
      USEABLE_STAND_ALONE "No"
      CONTAINS_CRYPTO "No"
      ECCN         "Unknown"
   )

endif(LSF_FOUND)
