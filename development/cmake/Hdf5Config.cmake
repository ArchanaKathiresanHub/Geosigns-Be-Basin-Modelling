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
# serial version
   set (HDF5_serial_HOME ${HPC_HOME}/hdf5-intel/${HDF5_VERSION}/LinuxRHEL64_x86_64_53WS CACHE PATH "Home for serial HDF5")
   set (HDF5_serial_FOUND TRUE)
   set (HDF5_serial_INCLUDE_DIR ${HDF5_serial_HOME}/include) 
   set (HDF5_serial_INCLUDE_DIRS ${HDF5_serial_INCLUDE_DIR}) 
   set (HDF5_hdf5_serial_LIBRARY ${HDF5_serial_HOME}/lib/libhdf5.a)
   list (APPEND HDF5_serial_LIBRARIES ${HDF5_hdf5_serial_LIBRARY})
   list (APPEND HDF5_serial_LIBRARIES z)

# parallel version
   set (HDF5_parallel_HOME ${HPC_HOME}/hdf5-intel-parallel/${HDF5_VERSION}/LinuxRHEL64_x86_64_53WS CACHE PATH "Home dir for parallel HDF5")
   set (HDF5_parallel_FOUND TRUE)
   set (HDF5_parallel_INCLUDE_DIR ${HDF5_parallel_HOME}/include)
   set (HDF5_parallel_INCLUDE_DIRS ${HDF5_parallel_INCLUDE_DIR})
   set (HDF5_hdf5_parallel_LIBRARY ${HDF5_parallel_HOME}/lib/libhdf5.a)
   list (APPEND HDF5_parallel_LIBRARIES ${HDF5_hdf5_parallel_LIBRARY})
   list (APPEND HDF5_parallel_LIBRARIES z)
   set(HDF5_parallel_DEFINITIONS "-DH5_HAVE_PARALLEL")
endif()

if(WIN32)
	set (HDF5_serial_HOME ${THIRD_PARTY_DIR}/hdf5.win${_64}/hdf5)
	# serial version
        set (HDF5_serial_FOUND TRUE)
	set (HDF5_serial_INCLUDE_DIR ${HDF5_serial_HOME}/include)
	set (HDF5_serial_INCLUDE_DIRS ${HDF5_serial_INCLUDE_DIR})
	set (HDF5_hdf5_serial_LIBRARY ${HDF5_serial_HOME}/lib/hdf5.lib )
	list (APPEND HDF5_serial_LIBRARIES ${HDF5_hdf5_serial_LIBRARY})
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
      
add_environment_path("${HDF5_serial_HOME}/bin")

