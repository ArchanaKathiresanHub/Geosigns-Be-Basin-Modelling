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

   set(HDF5_SOURCE_DIR "${CMAKE_BINARY_DIR}/hdf5" CACHE PATH "Source directory of HDF5")

# parallel version
   add_external_project_to_repository(
         NAME HDF5
         VERSION ${HDF5_VERSION}
         ARCHIVE "${THIRD_PARTY_DIR}/sources/hdf5-1.8.11.tar.gz"
         ARCHIVE_MD5 "3c28da19129fca00945d8eeea519db1c"
         CONFIGURE_COMMAND "./configure" "--prefix={ROOT}"
         BUILD_COMMAND   "make"
         INSTALL_COMMAND "make" "install"
         CONFIGURE_OPTIONS 
           COMPILER "{CurrentCompiler}"  "CC={CC}" "CXX={CXX}"
           MPI      "{CurrentMPI}"  "--enable-parallel"
           SPEED    "Release"    "--enable-production"
           SPEED    "Debug"      "--enable-production"
           SPEED    "DebugAll"   "--enable-debug"
           SPEED    "MemCheck"   "--enable-debug" "--enable-using-memchecker"
           OS       "{CurrentPlatform}"     
           LINK     "Dynamic"    "--enable-shared" "--disable-static"
           LINK     "Static"     "--disable-shared" "--enable-static"
         YIELD_LIBRARIES "hdf5" "hdf5_hl"
         YIELD_SOURCE "${HDF5_SOURCE_DIR}"
   )

   set(HDF5_INCLUDE_DIR ${HDF5_ROOT}/include)
   set(HDF5_INCLUDE_DIRS)
   list(APPEND HDF5_INCLUDE_DIRS ${HDF5_INCLUDE_DIR} ${MPI_INCLUDE_DIRS})
   set(HDF5_LIBRARIES "hdf5" "hdf5_hl" "z" ${BM_DL_LIB} )
  
else() # WIN32

   set (HDF5_ROOT "${THIRD_PARTY_DIR}/hdf5.win64-1.8.11/hdf5" CACHE PATH "Path to the HDF5 library")
   set (HDF5_FOUND TRUE)
   set (HDF5_INCLUDE_DIR ${HDF5_ROOT}/include)
   set (HDF5_INCLUDE_DIRS ${HDF5_INCLUDE_DIR} ${MPI_INCLUDE_DIRS})
   
   set(HDF5_LIB_ROOT "${HDF5_ROOT}/lib")
      if ( MSVC10 )
          set(HDF5_postfix "-msvc10")
      elseif(MSVC11)
          set(HDF5_postfix "-msvc11")
      elseif(MSVC12)
          set(HDF5_postfix "-msvc12")
      else()
          message(WARNING "Your Visual Studio version is not yet supported.")
      endif()
   set (HDF5_LIBRARY ${HDF5_LIB_ROOT}${HDF5_postfix}/libhdf5.lib CACHE FILEPATH "Location of HDF5 library")
   
   list (APPEND HDF5_LIBRARIES ${HDF5_LIBRARY} ${MPI_LIBRARIES})
   set(HDF5_SOURCE_DIR ${HDF5_ROOT} CACHE PATH "Source directory of HDF5")
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
    
