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

set(LSF_LIBS)
set(LSF_INCLUDE_DIR)

if(UNIX)

   set (LSF_HOME /apps/3rdparty/lsfnubo/lsf/9.1 CACHE PATH "Home dir for LSF")

   find_path(LSF_INCLUDE_DIR lsf/lsbatch.h
      PATHS
         $ENV{LSF_BINDIR}../../include
         ${LSF_HOME}/include
         $ENV{LSF_HOME}/include
   )

   find_library(LSBATCH_LIB NAMES bat
      PATHS 
         $ENV{LSF_LIBDIR}
         ${LSF_HOME}/lib
         $ENV{LSF_HOME}/lib
   )

   find_library(LSF_LIB NAMES lsf
      PATHS 
         $ENV{LSF_LIBDIR}
         ${LSF_HOME}/lib
         $ENV{LSF_HOME}/lib
   )

   find_library(NSL_LIB nsl)
   find_library(MATH_LIB m)

   if (LSF_INCLUDE_DIR AND LSF_LIB AND LSBATCH_LIB)
      set(LSF_FOUND "Yes")
      mark_as_advanced(LSF_INCLUDE_DIR)
      mark_as_advanced(LSBATCH_LIB)
      mark_as_advanced(LSF_LIB)

      list(APPEND LSF_LIBS "${LSBATCH_LIB}" "${LSF_LIB}" "${MATH_LIB}" "${NSL_LIB}")

      add_definitions(-DWITH_LSF_SCHEDULER)

   endif (LSF_INCLUDE_DIR AND LSF_LIB AND LSBATCH_LIB)

   # Set up LSF
   include(cmake/AddPackage.cmake)

   if (LSF_FOUND)
      add_external_package_info(
         CAPABILITY LSF
         NAME    "LSF"
         VENDOR  "IBM"
         VERSION "9.1.1"
         LICENSE_TYPE "Commercial"
         LICENSE_FILE "${LSF_HOME}/license/English.txt"
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

      add_environment_path("$ENV{LSF_BINDIR}")
   endif(LSF_FOUND)

   #Print status of LSF
   #MESSAGE( STATUS "LSF_FOUND "       ${LSF_FOUND} )
   #MESSAGE( STATUS "LSF_INCLUDE_DIR " ${LSF_INCLUDE_DIR} )
   #MESSAGE( STATUS "LSF_LIB "         ${LSF_LIB} )
   #MESSAGE( STATUS "LSF_LIBS "        ${LSF_LIBS} )

endif (UNIX)
