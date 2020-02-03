#########################################################################
#                                                                       #
# Copyright (C) 2012-2018 Shell International Exploration & Production. #
# All rights reserved.                                                  #
#                                                                       #
# Developed under license for Shell by PDS BV.                          #
#                                                                       #
# Confidential and proprietary source code of Shell.                    #
# Do not distribute without written permission from Shell.              #
#                                                                       #
#########################################################################

include_directories( SYSTEM
   ${HDF5_INCLUDE_DIRS}
)

set( LIB_NAME "genex6_kernel" )
create_bm_library( TARGET ${LIB_NAME}
LIBRARIES   AbstractDerivedProperties
   genex6
   CBMGenerics
   EosPack
   FileSystem
   utilities
   Serial_Hdf5
   TableIO
   OTGC_kernel6
   DataAccess
INSTALLTARGET
)

#######################################
### Unit Tests
#######################################

#######################################
# Unit tests for convertHItoHC

add_gtest ( NAME GENEX6_KERNEL::convertHItoHC
   SOURCES test/HiToHcTest.cpp 
   LIBRARIES genex6 genex6_kernel DataAccess DistributedDataAccess
   FOLDER "${BASE_FOLDER}/${LIB_NAME}"
 )
 add_gtest ( NAME GENEX6_KERNEL::checkTargetHCValues
   SOURCES test/GenexSourceRockTest.cpp
   INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src
   LIBRARIES genex6 genex6_kernel DataAccess SerialDataAccess
   FOLDER "${BASE_FOLDER}/${LIB_NAME}"
 )
