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

FILE( GLOB all_headers src/Interface/*.h )
source_group(include FILES ${all_headers})

FILE( GLOB all_srcs src/*.C)
source_group(source FILES ${all_srcs})

set( LIB_NAME "SerialDataAccess" )
add_library(${LIB_NAME} 
   ${all_srcs}
   ${all_headers}
)
set_target_properties( ${LIB_NAME} PROPERTIES FOLDER "${BASE_FOLDER}/${LIB_NAME}" )

bm_include_libraries(
   DataModel
   Serial_Hdf5
   utilities
   FileSystem 
   TableIO
   CBMGenerics
   DataAccess
)

include_directories( SYSTEM
   ${HDF5_INCLUDE_DIRS}
)

generate_dox( src/SerialDataAccess.cfg )

target_link_libraries(${LIB_NAME}
   Serial_Hdf5
   CBMGenerics
   utilities
   FileSystem
   TableIO
   DataAccess
)

if (UNIX)
   target_link_libraries(${LIB_NAME} rt)
endif(UNIX)

install(TARGETS ${LIB_NAME}
   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)

add_gtest( NAME SerialGridMap
           SOURCES test/SerialGridMap.C
           LIBRARIES ${LIB_NAME} DataAccess
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME SerialMapWriter
           SOURCES test/SerialMapWriter.C
           LIBRARIES ${LIB_NAME} DataAccess Serial_Hdf5 FileSystem
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)
