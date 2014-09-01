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

include(cmake/AddPackage.cmake)

set (HDF5_VERSION  "1.8.11" CACHE STRING "HDF5 version")

if(UNIX)

# parallel version
   set (HDF5_HOME ${HPC_HOME}/hdf5-intel-parallel/${HDF5_VERSION}/LinuxRHEL64_x86_64_53WS CACHE PATH "Home dir for parallel HDF5")
   set (HDF5_FOUND TRUE)

   set (HDF5_INCLUDE_DIR ${HDF5_HOME}/include)
   set (HDF5_INCLUDE_DIRS)
   list (APPEND HDF5_INCLUDE_DIRS ${HDF5_INCLUDE_DIR} ${MPI_INCLUDE_DIRS})

   find_library(HDF5_BASE_LIBRARY hdf5 PATHS "${HDF5_HOME}/lib" NO_DEFAULT_PATH)
   find_library(HDF5_HL_LIBRARY hdf5_hl PATHS "${HDF5_HOME}/lib" NO_DEFAULT_PATH)
   list (APPEND HDF5_LIBRARIES ${HDF5_HL_LIBRARY} ${HDF5_BASE_LIBRARY} z ${MPI_LIBRARIES})

   # HDF5 Source
   set(HDF5_SOURCE_DIR "${HPC_HOME}/hdf5-src/${HDF5_VERSION}" CACHE PATH "Source directory of HDF5")

   add_environment_path("${HDF5_HOME}/bin")

else() # WIN32

   set (HDF5_HOME ${THIRD_PARTY_DIR}/hdf5.win64-1.8.11/hdf5)
      set (HDF5_FOUND TRUE)
   set (HDF5_INCLUDE_DIR ${HDF5_HOME}/include)
   set (HDF5_INCLUDE_DIRS ${HDF5_INCLUDE_DIR} ${MPI_INCLUDE_DIRS})
   
   set(HDF5_LIB_ROOT "${HDF5_HOME}/lib")
      if ( MSVC10 )
          set(HDF5_postfix "-msvc10")
      elseif(MSVC11)
          set(HDF5_postfix "-msvc11")
      elseif(MSVC12)
          set(HDF5_postfix "-msvc12")
      else()
          message( FATAL_ERROR "No sutitable version of HDF5 binaries were found." )
      endif()
   set (HDF5_LIBRARY ${HDF5_LIB_ROOT}${HDF5_postfix}/libhdf5.lib )
   
   list (APPEND HDF5_LIBRARIES ${HDF5_LIBRARY} ${MPI_LIBRARIES})
      set(HDF5_SOURCE_DIR ${HDF5_HOME} CACHE PATH "Source directory of HDF5")

endif()

add_external_package_info(
      CAPABILITY HDF5
      NAME    "HDF5"
      VENDOR  "The HDF Group"
      VERSION "${HDF5_VERSION}"
      LICENSE_TYPE "BSD"
      LICENSE_FILE "${THIRD_PARTY_DIR}/licenses/Hdf5-${HDF5_VERSION}.txt"
      URL "http://www.hdfgroup.org/HDF5/"
      DESCRIPTION "Data model, library, and file format for storing and managing data in scientific applications"
      REQUIRED_AT  "Runtime"
      COUNTRY_OF_ORIGIN "USA"
      SHIPPED      "Yes"
      INCLUSION_TYPE "Static Link"
      USEABLE_STAND_ALONE "No"
      CONTAINS_CRYPTO "No"
      ECCN         "EAR99"
)
      

