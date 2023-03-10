#########################################################################
#                                                                       #
# Copyright (C) 2012-2018 Shell International Exploration & Production. #
# All rights reserved.                                                  #
#                                                                       #
# Confidential and proprietary source code of Shell.                    #
# Do not distribute without written permission from Shell.              #
#                                                                       #
#########################################################################

set( LIB_NAME "DataExtraction" )

include_directories( SYSTEM
   ${HDF5_INCLUDE_DIRS}
   )

create_bm_library( TARGET ${LIB_NAME}
   LIBRARIES
   FiniteElements
   utilities
   Interpolation
   LinearAlgebra
   TableIO
   EosPack
   AbstractDerivedProperties
   DerivedProperties
   DataAccess
   GeoPhysics
   CBMGenerics
   DataMining
   ${HDF5_LIBRARIES}
   )

################################################################
######################## UNIT TESTS ############################
################################################################

add_gtest( NAME "Track1dParseUtilities"
   SOURCES test/parseUtilitiesTest.cpp src/parseUtilities.cpp
   FOLDER "${BASE_FOLDER}/${LIB_NAME}"
   )

add_gtest( NAME "hdfReadManagerTest"
   LIBRARIES DataMining DataAccess SerialDataAccess ${HDF5_LIBRARIES}
   SOURCES test/hdfReadManagerTest.cpp src/hdfReadManager.cpp
   FOLDER "${BASE_FOLDER}/${LIB_NAME}"
   )

configure_file( test/Time_0.000000.h5 Time_0.000000.h5 COPYONLY )
configure_file( test/Project.project3d Project.project3d COPYONLY )
