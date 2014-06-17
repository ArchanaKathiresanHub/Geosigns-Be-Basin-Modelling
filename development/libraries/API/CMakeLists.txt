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

FILE( GLOB all_headers src/*.h )
source_group(include FILES ${all_headers})

FILE( GLOB all_srcs src/*.C)
source_group(source FILES ${all_srcs})

add_library(api
	${all_srcs}
	${all_headers}
)

bm_include_libraries(
   CBMGenerics
   Serial_Hdf5
   TableIO
   DataAccess
)

include_directories(
	${SUMPP_INCLUDE_DIR}
	${HDF5_INCLUDE_DIRS} 
	src
)

target_link_libraries(api 
   CBMGenerics
   Serial_Hdf5
   TableIO
   DataAccess
   SUMlib
)

generate_dox( src/API.cfg )
	
#######################################
### Unit Tests
#######################################

#######################################
# Unit tests for DoEGeneratorTest

configure_file(test/Project.project3d Project.project3d) 
configure_file(test/Ottoland.project3d Ottoland.project3d) 

add_gtest( NAME BLRSTest
   SOURCES test/BLRSTest.C
   LIBRARIES api DataAccess SerialDataAccess TableIO ${HDF5_LIBRARIES}
)

add_gtest( NAME DoETest
   SOURCES test/DoETest.C
   LIBRARIES api DataAccess SerialDataAccess TableIO ${HDF5_LIBRARIES}
)

add_gtest( NAME mbapiModelTest
   SOURCES test/mbapiModelTest.C
   LIBRARIES api DataAccess SerialDataAccess TableIO ${HDF5_LIBRARIES}
)
