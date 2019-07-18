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

set( LIB_NAME "cmbapi" )

include_directories( SYSTEM
   ${HDF5_INCLUDE_DIRS}
   ${NNLIB_INCLUDE_DIR}
)

create_bm_library( TARGET ${LIB_NAME}
                   LIBRARIES GeoPhysics
                             SerialDataAccess
                             TableIO
                             Serial_Hdf5
                             genex6
                             genex6_kernel
                             DataAccess
                             EosPack
                             CBMGenerics
                             FileSystem
                             utilities
                             NNlib
                             ${Boost_LIBRARIES} )

if (UNIX)
  add_dependencies(${LIB_NAME} NNlib)
endif (UNIX)

generate_dox( src/API.cfg )
   
#######################################
### Unit Tests
#######################################

#######################################
# Unit tests for DoEGeneratorTest

#Various cauldron projects which are used in unit tests
copy_test_file(Project.project3d)
copy_test_file(SourceRockTesting.project3d)
copy_test_file(LithologyTesting.project3d)
copy_test_file(DupLithologyTesting.project3d)
copy_test_file(FluidTesting.project3d)
copy_test_file(CtcTesting.project3d)
copy_test_file(FracturePressureTesting.project3d)
copy_test_file(ReservoirTesting.project3d)
copy_test_file(MapsTesting.project3d)
copy_test_file(Inputs.HDF)
copy_test_file(Inputs_ctc.HDF)
copy_test_file(Inputs_FracPressure.HDF)
copy_test_file(NNTesting.project3d)
copy_test_file(NNInt)
copy_test_file(NNInputLithofractions)
copy_test_file(NNbt)
copy_test_file(TopBoundaryTesting.project3d)
configure_file(../casaAPI/test/Ottoland.project3d Ottoland.project3d COPYONLY )


add_gtest( NAME mbapiModelTest
   SOURCES test/mbapiModelTest.cpp
   LIBRARIES ${LIB_NAME} TableIO DataAccess SerialDataAccess
   FOLDER "${BASE_FOLDER}/${LIB_NAME}" )
