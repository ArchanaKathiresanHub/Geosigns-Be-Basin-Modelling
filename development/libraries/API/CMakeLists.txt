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

configure_file(src/CauldronEnvConfig.h.in CauldronEnvConfig.h)

add_library(api
	${all_srcs}
	${all_headers}
)

bm_include_libraries(
   CBMGenerics
   Serial_Hdf5
   TableIO
   DataAccess
   utilities
)

include_directories(
   ${SUMPP_INCLUDE_DIR}
   ${HDF5_INCLUDE_DIRS} 
   ${LSF_INCLUDE_DIR}
   ${CMAKE_CURRENT_BINARY_DIR}
   src
)

target_link_libraries(api 
   CBMGenerics
   Serial_Hdf5
   TableIO
   DataAccess
   utilities
   SUMlib
   ${LSF_LIBS}
)

generate_dox( src/API.cfg )
	
#######################################
### Unit Tests
#######################################

#######################################
# Unit tests for DoEGeneratorTest

#Various cauldron projects which are used in unit tests
configure_file(test/Project.project3d Project.project3d) 
configure_file(test/Ottoland.project3d Ottoland.project3d) 

#Cauldron project with results for DataDigger unit testing
configure_file(test/OttolandWithGenex.project3d OttolandWithGenex.project3d) 
configure_file(test/OttolandWithGenex_CauldronOutputDir/Time_0.000000.h5 OttolandWithGenex_CauldronOutputDir/Time_0.000000.h5) 
configure_file(test/OttolandWithGenex_CauldronOutputDir/Genex5_Results.HDF OttolandWithGenex_CauldronOutputDir/Genex5_Results.HDF) 
configure_file(test/OttolandWithGenex_CauldronOutputDir/PressureAndTemperature_Results.HDF OttolandWithGenex_CauldronOutputDir/PressureAndTemperature_Results.HDF) 

add_gtest( NAME BLRSTest
   SOURCES test/BLRSTest.C
   LIBRARIES api DataAccess SerialDataAccess TableIO ${HDF5_LIBRARIES} ${Boost_LIBRARIES}
)

add_gtest( NAME DoETest
   SOURCES test/DoETest.C
   LIBRARIES api DataAccess SerialDataAccess TableIO ${HDF5_LIBRARIES} ${Boost_LIBRARIES}
)

add_gtest( NAME MutatorTest
   SOURCES test/MutatorTest.C
   LIBRARIES api DataAccess SerialDataAccess TableIO ${HDF5_LIBRARIES} ${Boost_LIBRARIES}
)

add_gtest( NAME ValidatorTest
   SOURCES test/ValidatorTest.C
   LIBRARIES api DataAccess SerialDataAccess TableIO ${HDF5_LIBRARIES} ${Boost_LIBRARIES}
)

add_gtest( NAME mbapiModelTest
   SOURCES test/mbapiModelTest.C
   LIBRARIES api DataAccess SerialDataAccess TableIO ${HDF5_LIBRARIES} ${Boost_LIBRARIES}
)

add_gtest( NAME RunManagerTest
   SOURCES test/RunManagerTest.C
   INCLUDE_DIRS ${LSF_INCLUDE_DIR}
   LIBRARIES api DataAccess SerialDataAccess TableIO ${HDF5_LIBRARIES} ${Boost_LIBRARIES} ${LSF_LIBS}
)

add_gtest( NAME DataDiggerTest
   SOURCES test/DataDiggerTest.C
   LIBRARIES api DataAccess SerialDataAccess TableIO ${HDF5_LIBRARIES} ${Boost_LIBRARIES}
)
