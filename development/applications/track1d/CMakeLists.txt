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

set( TARGET_NAME "track1d" )
add_executable(${TARGET_NAME} src/main.cpp)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER "${BASE_FOLDER}/${TARGET_NAME}" )

include_directories( SYSTEM
   ${HDF5_INCLUDE_DIRS}
)

target_link_libraries(${TARGET_NAME}
   AbstractDerivedProperties
   DataExtraction
   DerivedProperties
   DataMining
   GeoPhysics
   DataAccess
   SerialDataAccess
   Interpolation
   LinearAlgebra
   TableIO
   EosPack
   CBMGenerics
   Serial_Hdf5
   utilities
   ${HDF5_LIBRARIES}
)

install(TARGETS ${TARGET_NAME}
   RUNTIME DESTINATION bin
)
