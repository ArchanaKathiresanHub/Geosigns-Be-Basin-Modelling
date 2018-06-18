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

set( LIB_NAME "cmbapi" )
add_library(${LIB_NAME}
   ${all_srcs}
   ${all_headers}
)
set_target_properties( ${LIB_NAME} PROPERTIES FOLDER "${BASE_FOLDER}/${LIB_NAME}" )

if (UNIX)
add_dependencies(${LIB_NAME} NNlib)
endif (UNIX)

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
   src
)

include_directories( SYSTEM
   ${HDF5_INCLUDE_DIRS}
   ${NNLIB_INCLUDE_DIR}
)

target_link_libraries(${LIB_NAME}
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
   NNlib
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
configure_file(test/DupLithologyTesting.project3d DupLithologyTesting.project3d COPYONLY )
configure_file(test/FluidTesting.project3d FluidTesting.project3d COPYONLY )
configure_file(../casaAPI/test/Ottoland.project3d Ottoland.project3d COPYONLY )
configure_file( test/MapsTesting.project3d MapsTesting.project3d COPYONLY )
configure_file( test/Inputs.HDF Inputs.HDF COPYONLY )
configure_file( test/NNTesting.project3d NNTesting.project3d COPYONLY )
configure_file( test/NNInt NNInt COPYONLY )
configure_file( test/NNInputLithofractions NNInputLithofractions COPYONLY )
configure_file( test/NNbt NNbt COPYONLY )


add_gtest( NAME mbapiModelTest
   SOURCES test/mbapiModelTest.cpp
   LIBRARIES ${LIB_NAME} TableIO DataAccess SerialDataAccess
   FOLDER "${BASE_FOLDER}/${LIB_NAME}" )
