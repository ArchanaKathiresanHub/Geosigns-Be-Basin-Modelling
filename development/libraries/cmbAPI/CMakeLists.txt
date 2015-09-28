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

FILE( GLOB all_headers src/*.h )
source_group(include FILES ${all_headers})

FILE( GLOB all_srcs src/*.cpp)
source_group(source FILES ${all_srcs})

add_library(cmbapi
	${all_srcs}
	${all_headers}
)

bm_include_libraries(
   genex6
   genex6_kernel
   EosPack
   DataModel
   CBMGenerics
   Serial_Hdf5
   DataAccess
   TableIO
   utilities
   FileSystem
)

include_directories(
   ${HDF5_INCLUDE_DIRS}
   src
)

target_link_libraries(cmbapi
   CBMGenerics
   utilities
   TableIO
   Serial_Hdf5
   DataAccess
   GeoPhysics
   SerialDataAccess
   genex6
   genex6_kernel
   EosPack
   FileSystem
   ${Boost_LIBRARIES}
)

generate_dox( src/API.cfg )
	
#######################################
### Unit Tests
#######################################

#######################################
# Unit tests for DoEGeneratorTest

#Various cauldron projects which are used in unit tests
configure_file(test/Project.project3d Project.project3d COPYONLY )
configure_file(test/SourceRockTesting.project3d SourceRockTesting.project3d COPYONLY )
configure_file(test/LithologyTesting.project3d LithologyTesting.project3d COPYONLY )
configure_file(../casaAPI/test/Ottoland.project3d Ottoland.project3d COPYONLY )
configure_file( test/MapsTesting.project3d MapsTesting.project3d COPYONLY )
configure_file( test/Inputs.HDF Inputs.HDF COPYONLY )


add_gtest( NAME mbapiModelTest
   SOURCES test/mbapiModelTest.cpp
   LIBRARIES cmbapi TableIO DataAccess SerialDataAccess
)
