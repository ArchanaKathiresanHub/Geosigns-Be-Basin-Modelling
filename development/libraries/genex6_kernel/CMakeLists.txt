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

set( LIB_NAME "genex6_kernel" )
add_library(${LIB_NAME} 
   ${all_srcs}
   ${all_headers}
)
set_target_properties( ${LIB_NAME} PROPERTIES FOLDER "${BASE_FOLDER}/${LIB_NAME}" )

bm_include_libraries(
   DataModel
   genex6 
   CBMGenerics 
   EosPack 
   DerivedProperties
   GeoPhysics
   Interpolation
   LinearAlgebra
   FileSystem
   utilities 
   Serial_Hdf5 
   OTGC_kernel6
   TableIO 
   DataAccess 
   DataAccessInterface
)

include_directories(
   ..
   src
)

include_directories( SYSTEM
   ${HDF5_INCLUDE_DIRS}
)

target_link_libraries(${LIB_NAME}
   genex6
   CBMGenerics
   EosPack
   DerivedProperties
   FileSystem
   utilities
   Serial_Hdf5
   OTGC_kernel6
   TableIO
   DataAccess
)

install(TARGETS ${LIB_NAME}
   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)

