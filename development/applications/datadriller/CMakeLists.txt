#########################################################################
#                                                                       #
# Copyright (C) 2012-2018 Shell International Exploration & Production. #
# All rights reserved.                                                  #
#                                                                       #
# Confidential and proprietary source code of Shell.                    #
# Do not distribute without written permission from Shell.              #
#                                                                       #
#########################################################################

set( TARGET_NAME "datadriller" )
add_executable( ${TARGET_NAME} src/main.C )
set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER "${BASE_FOLDER}/${TARGET_NAME}" )

include_directories( SYSTEM
   ${HDF5_INCLUDE_DIRS}
)

target_link_libraries(${TARGET_NAME}
   DataExtraction
   DataMining
   DataAccess
   SerialDataAccess
   CBMGenerics
   GeoPhysics
   AbstractDerivedProperties
   DerivedProperties
   Serial_Hdf5
   EosPack
   utilities
   TableIO
   LinearAlgebra
   Interpolation
   FiniteElements
   ${HDF5_LIBRARIES}
)

set_target_properties( ${TARGET_NAME}
      PROPERTIES LINK_FLAGS "${PETSC_LINK_FLAGS}"
)

install(TARGETS ${TARGET_NAME}
   RUNTIME DESTINATION bin
)
