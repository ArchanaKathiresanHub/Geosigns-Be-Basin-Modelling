#########################################################################
#                                                                       #
# Copyright (C) 2012-2018 Shell International Exploration & Production. #
# All rights reserved.                                                  #
#                                                                       #
# Confidential and proprietary source code of Shell.                    #
# Do not distribute without written permission from Shell.              #
#                                                                       #
#########################################################################

################################################################
######################### SET TARGETS ##########################
################################################################

FILE( GLOB all_headers src/*.h )
source_group(include FILES ${all_headers})

FILE( GLOB all_srcs src/*.C src/*.cpp)
source_group(source FILES ${all_srcs})

set( TARGET_NAME "mapMorph" )
add_executable( ${TARGET_NAME}
   ${all_srcs}
   ${all_headers}
)
set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER "${BASE_FOLDER}/${TARGET_NAME}" )

################################################################
################### SET DEPENDENCIES/LINKS #####################
################################################################

include_directories(
   ${HDF5_INCLUDE_DIRS}
)

target_link_libraries( ${TARGET_NAME}
   cmbapi
   FileSystem
   TableIO
   DataModel
   DataAccess
   utilities
   ${HDF5_LIBRARIES}
)

install( TARGETS ${TARGET_NAME}
         DESTINATION bin
)

################################################################
######################## UNIT TESTS ############################
################################################################

configure_file( test/wellData.in wellData.in COPYONLY )

add_gtest( NAME ${TARGET_NAME}::WellReaderTest
  SOURCES test/wellReaderTest.cpp src/wellReader.cpp
  LIBRARIES FileSystem
  FOLDER "${BASE_FOLDER}/${TARGET_NAME}"
)
