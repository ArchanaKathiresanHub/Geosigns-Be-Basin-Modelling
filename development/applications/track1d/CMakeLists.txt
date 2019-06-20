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

#FILE( GLOB all_headers src/*.h )
#FILE( GLOB all_srcs src/*.C)
set (all_headers,
   src/errorhandling.h
   src/FormationMapOutputPropertyValue.h
   src/FormationOutputPropertyValue.h
   src/FormationSurfaceOutputPropertyValue.h
   src/OutputPropertyValue.h
   src/SurfaceOutputPropertyValue.h)

set(all_srcs
   src/FormationMapOutputPropertyValue.C
   src/FormationOutputPropertyValue.C
   src/FormationSurfaceOutputPropertyValue.C
   src/SurfaceOutputPropertyValue.C
   src/track1d.C)

source_group(include FILES ${all_headers})

source_group(source FILES ${all_srcs})

set( TARGET_NAME "track1d" )
add_executable(${TARGET_NAME} 
   ${all_srcs}
   ${all_headers}
)
set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER "${BASE_FOLDER}/${TARGET_NAME}" )

include_directories( SYSTEM
   ${HDF5_INCLUDE_DIRS}
)

target_link_libraries(${TARGET_NAME}
   AbstractDerivedProperties
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
