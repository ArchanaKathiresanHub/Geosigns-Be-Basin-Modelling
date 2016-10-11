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

FILE( GLOB all_srcs src/*.cpp)
source_group(source FILES ${all_srcs})

configure_file(src/CauldronEnvConfig.h.in CauldronEnvConfig.h)

add_library(casaapi
   ${all_srcs}
   ${all_headers}
)

if (UNIX)
add_dependencies(casaapi Boost)
endif (UNIX)


bm_include_libraries(
   CBMGenerics
   TableIO
   cmbAPI
   FileSystem
   utilities
   DataModel
   DataAccess
   EosPack
)

include_directories(
   ${SUMPP_INCLUDE_DIR}
   ${CMAKE_CURRENT_BINARY_DIR}
   ${EIGEN_INCLUDE_DIRS}
   ${NNLIB_INCLUDE_DIR}
   src
)

target_link_libraries(casaapi
   TableIO
   cmbapi
   FileSystem
   utilities
   DataAccess
   EosPack
   SUMlib
   NNlib
   ${Boost_LIBRARIES}
)

if (LSF_HOME)
   include_directories(SYSTEM ${LSF_INCLUDE_DIR})
   target_link_libraries(casaapi ${LSF_LIBS})
endif(LSF_HOME)

generate_dox( src/API.cfg )

#######################################
### Unit Tests
#######################################

#######################################
# Unit tests for DoEGeneratorTest

#Various cauldron projects which are used in unit tests
configure_file(test/Ottoland.project3d Ottoland.project3d)

#Cauldron project with results for DataDigger unit testing
configure_file(test/OttolandWithGenex.project3d OttolandWithGenex.project3d COPYONLY )
configure_file(test/OttolandCatPrms.project3d OttolandCatPrms.project3d COPYONLY )
configure_file(test/Ottoland_casa_state.txt Ottoland_casa_state.txt COPYONLY )
configure_file(test/LithologyTesting.project3d LithologyTesting.project3d COPYONLY )

# Generate best match case test
configure_file(test/BestMatchCaseGeneration_state.txt BestMatchCaseGeneration_state.txt COPYONLY)
configure_file(test/NVG_Project.project3d NVG_Project.project3d COPYONLY)
configure_file(test/MAP-72981789-4.FLT MAP-72981789-4.FLT COPYONLY)
configure_file(test/Inputs.HDF Inputs.HDF COPYONLY)


add_gtest( NAME BLRSTest
   SOURCES test/BLRSTest.cpp
   LIBRARIES casaapi cmbapi DataAccess SerialDataAccess TableIO ${HDF5_LIBRARIES} ${Boost_LIBRARIES}
)

add_gtest( NAME DoETest
   SOURCES test/DoETest.cpp
   LIBRARIES casaapi cmbapi DataAccess SerialDataAccess TableIO ${HDF5_LIBRARIES} ${Boost_LIBRARIES}
)

add_gtest( NAME MutatorTest
   SOURCES test/MutatorTest.cpp
   LIBRARIES casaapi cmbapi DataAccess SerialDataAccess TableIO ${HDF5_LIBRARIES} ${Boost_LIBRARIES}
)

add_gtest( NAME ValidatorTest
   SOURCES test/ValidatorTest.cpp
   LIBRARIES casaapi cmbapi DataAccess SerialDataAccess TableIO ${HDF5_LIBRARIES} ${Boost_LIBRARIES}
)

if (LSF_HOME)
   add_gtest( NAME RunManagerTest
      SOURCES test/RunManagerTest.cpp
      INCLUDE_DIRS ${LSF_INCLUDE_DIR}
      LIBRARIES casaapi cmbapi DataAccess SerialDataAccess TableIO ${HDF5_LIBRARIES} ${Boost_LIBRARIES} ${LSF_LIBS}
   )
endif()

add_gtest( NAME DataDiggerTest
   SOURCES test/DataDiggerTest.cpp
   LIBRARIES casaapi cmbapi DataAccess SerialDataAccess TableIO ${HDF5_LIBRARIES} ${Boost_LIBRARIES}
)

add_gtest( NAME RSProxyTest
   SOURCES test/RSProxyTest.cpp
   LIBRARIES casaapi cmbapi DataAccess SerialDataAccess TableIO ${HDF5_LIBRARIES} ${Boost_LIBRARIES}
)

add_gtest( NAME MCTest
   SOURCES test/MCTest.cpp
   LIBRARIES casaapi cmbapi DataAccess SerialDataAccess TableIO ${HDF5_LIBRARIES} ${Boost_LIBRARIES}
)

add_gtest( NAME SerializationTest
   SOURCES test/SerializationTest.cpp
   LIBRARIES casaapi cmbapi DataAccess SerialDataAccess TableIO ${HDF5_LIBRARIES} ${Boost_LIBRARIES}
)

add_gtest( NAME BestMatchedCaseGenerationTest
   SOURCES test/BestMatchedCaseGenerationTest.cpp
   LIBRARIES casaapi cmbapi DataAccess SerialDataAccess TableIO ${HDF5_LIBRARIES} ${Boost_LIBRARIES}
)

add_gtest( NAME SensCalcTest
   SOURCES test/SensCalcTest.cpp
   LIBRARIES casaapi cmbapi DataAccess SerialDataAccess TableIO ${HDF5_LIBRARIES} ${Boost_LIBRARIES}
)
# temporary disabled, under development
#add_gtest( NAME LMOptimTest
#   SOURCES test/LMOptimTest.cpp
#   LIBRARIES casaapi cmbapi DataAccess SerialDataAccess TableIO ${HDF5_LIBRARIES} ${Boost_LIBRARIES}
#)
