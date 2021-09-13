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


if(UNIX)
   set(HDF5_SOURCE_DIR "${CMAKE_BINARY_DIR}/hdf5" CACHE PATH "Source directory of HDF5")

# parallel version
   add_external_project_to_repository(
         NAME HDF5
         VERSION ${HDF5_VERSION}
         ARCHIVE "${THIRD_PARTY_DIR}/sources/hdf5-${HDF5_VERSION}.tar.gz"
         ARCHIVE_MD5 "9e22217d22eb568e09f0cc15fb641d7c"
         CONFIGURE_COMMAND "./configure" "--prefix={ROOT}"
         BUILD_COMMAND   "make"
         INSTALL_COMMAND "make" "install"
         CONFIGURE_OPTIONS 
           COMPILER "{CurrentCompiler}"  "CC={CC}" "CXX={CXX}"
           MPI      "{CurrentMPI}"  "--enable-parallel"
           SPEED    "Release"      "--enable-build-mode=production" "CFLAGS=-O2" "CXXFLAGS=-O2"
           SPEED    "Debug"        "--enable-build-mode=debug" "CFLAGS=-O0" "CXXFLAGS=-O0"
           SPEED    "DebugAll"     "--enable-build-mode=debug" "CFLAGS=-O0" "CXXFLAGS=-O0"
           SPEED    "MemCheck"     "--enable-build-mode=debug" "--enable-using-memchecker" "CFLAGS=-O0" "CXXFLAGS=-O0"
           SPEED    "CodeCoverage" "--enable-build-mode=production" "CFLAGS=-O2" "CXXFLAGS=-O2"
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
#   find_package (HDF5 NAMES hdf5 COMPONENTS C static) 
   
  set(HDF5_INCLUDE_DIR ${HDF5_ROOT}/include)
  set(HDF5_INCLUDE_DIRS ${HDF5_INCLUDE_DIR} ${MPI_INCLUDE_DIRS})
  
  set(HDF5_LIB_ROOT "${HDF5_ROOT}/lib")
  set(HDF5_LIBRARY ${HDF5_LIB_ROOT}/libhdf5.lib CACHE FILEPATH "Location of HDF5 library")
  
  list(APPEND HDF5_LIBRARIES ${HDF5_LIBRARY} ${MPI_LIBRARIES})
  set( HDF5_SOURCE_DIR ${HDF5_INCLUDE_DIR}/.. CACHE PATH "Source directory of HDF5" )
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
  
