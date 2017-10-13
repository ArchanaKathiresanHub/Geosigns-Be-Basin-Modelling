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
FILE( GLOB all_headers src/*.h )
source_group(include FILES ${all_headers})

FILE( GLOB all_srcs src/*.C)
source_group(source FILES ${all_srcs})

set( LIB_NAME "DataMining" )
add_library(${LIB_NAME}
   ${all_srcs}
   ${all_headers}
)
set_target_properties( ${LIB_NAME} PROPERTIES FOLDER "${BASE_FOLDER}/${LIB_NAME}" )

bm_include_libraries( 
   DataModel
   FiniteElements 
   utilities 
   Interpolation 
   LinearAlgebra 
   DerivedProperties
   TableIO 
   EosPack 
   DataAccess 
   GeoPhysics 
   CBMGenerics
)

include_directories(
   src
)

include_directories( SYSTEM
   ${HDF5_INCLUDE_DIRS}
)

target_link_libraries(${LIB_NAME}
   FiniteElements
   utilities
   Interpolation
   LinearAlgebra
   TableIO
   EosPack
   DerivedProperties
   DataAccess
   GeoPhysics
   CBMGenerics
   ${HDF5_LIBRARIES}
)

install(TARGETS ${LIB_NAME}
   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)

add_gtest( NAME DataMiningProjectHandle
           SOURCES test/DataMiningProjectHandle.C
           LIBRARIES ${LIB_NAME} DataAccess SerialDataAccess
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

configure_file(
   ${CMAKE_CURRENT_SOURCE_DIR}/test/DataMiningProjectHandleTest.project3d
   DataMiningProjectHandleTest.project3d
   COPYONLY
)

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
