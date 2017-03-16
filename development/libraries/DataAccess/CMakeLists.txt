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

FILE( GLOB all_headers src/*.h src/Interface/*.h )
source_group(include FILES ${all_headers})

FILE( GLOB all_srcs src/*.C)
source_group(source FILES ${all_srcs})

add_library(DataAccess 
   ${all_srcs}
   ${all_headers}
)

bm_include_libraries(
   DataModel
   Serial_Hdf5
   CBMGenerics
   utilities
   FileSystem 
   TableIO
   EosPack
   
   CrustalThickness
)

include_directories(
  src
)

include_directories( SYSTEM
   ${HDF5_INCLUDE_DIRS}
)

target_link_libraries(DataAccess
   ${Boost_LIBRARIES}
   Serial_Hdf5
   CBMGenerics
   FileSystem
   utilities
   TableIO
)

if (UNIX)
add_dependencies(DataAccess HDF5)
endif (UNIX)

add_gtest( NAME DataAccess
           SOURCES test/DataAccess.C
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../SerialDataAccess/src/Interface/
           LIBRARIES DataAccess SerialDataAccess Serial_Hdf5
           ENV_VARS EOSPACKDIR=${CFGFLS}/eospack GENEX5DIR=${CFGFLS}/genex50 CTCDIR=${CFGFLS}/
)

configure_file(
   ${CMAKE_CURRENT_SOURCE_DIR}/test/DataAccessTest.project3d
   DataAccessTest.project3d
   COPYONLY
)

generate_dox( src/DataAccess.cfg )

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
