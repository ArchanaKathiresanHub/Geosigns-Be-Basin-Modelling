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

set( LIB_NAME "DataMining" )

include_directories( SYSTEM
   ${HDF5_INCLUDE_DIRS}
)

create_bm_library( TARGET ${LIB_NAME}
                   LIBRARIES DerivedProperties
                             GeoPhysics
                             FiniteElements
                             Interpolation
                             LinearAlgebra
                             TableIO
                             EosPack
                             AbstractDerivedProperties
                             DataAccess
                             utilities
                             ${HDF5_LIBRARIES}
                   INSTALLTARGET )

add_gtest( NAME DataMiningProjectHandle
           SOURCES test/DataMiningProjectHandle.C
           LIBRARIES ${LIB_NAME} DataAccess SerialDataAccess
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

copy_test_file(DataMiningProjectHandleTest.project3d)
