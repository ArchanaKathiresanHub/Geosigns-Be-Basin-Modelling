#########################################################################
#                                                                       #
# Copyright (C) 2012-2017 Shell International Exploration & Production. #
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

add_library(casalib
   ${all_srcs}
   ${all_headers}
)

if (UNIX)
add_dependencies(casalib Boost)
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
   src
)

include_directories( SYSTEM
   ${SUMPP_INCLUDE_DIR}
   ${CMAKE_CURRENT_BINARY_DIR}
   ${EIGEN_INCLUDE_DIRS}
   ${NNLIB_INCLUDE_DIR}
)

target_link_libraries(casalib
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
   target_link_libraries(casalib ${LSF_LIBS})
endif(LSF_HOME)

generate_dox( src/API.cfg )

#######################################
### Unit Tests
#######################################

#######################################
# Unit tests for DoEGeneratorTest

#Various cauldron projects which are used in unit tests
configure_file(test/Ottoland.project3d ${CMAKE_CURRENT_BINARY_DIR}/Ottoland.project3d)

#Cauldron project with results for DataDigger unit testing
configure_file(test/OttolandWithGenex.project3d ${CMAKE_CURRENT_BINARY_DIR}/OttolandWithGenex.project3d COPYONLY )
configure_file(test/OttolandCatPrms.project3d   ${CMAKE_CURRENT_BINARY_DIR}/OttolandCatPrms.project3d   COPYONLY )
configure_file(test/Ottoland_casa_state.txt     ${CMAKE_CURRENT_BINARY_DIR}/Ottoland_casa_state.txt )
configure_file(test/LithologyTesting.project3d  ${CMAKE_CURRENT_BINARY_DIR}/LithologyTesting.project3d  COPYONLY )

# Generate best match case test
configure_file(test/BestMatchCaseGeneration_state.txt ${CMAKE_CURRENT_BINARY_DIR}/BestMatchCaseGeneration_state.txt )
configure_file(test/NVG_Project.project3d             ${CMAKE_CURRENT_BINARY_DIR}/NVG_Project.project3d COPYONLY)
configure_file(test/MAP-72981789-4.FLT                ${CMAKE_CURRENT_BINARY_DIR}/MAP-72981789-4.FLT COPYONLY)
configure_file(test/Inputs.HDF                        ${CMAKE_CURRENT_BINARY_DIR}/Inputs.HDF COPYONLY)


add_gtest( NAME BLRSTest
   SOURCES test/BLRSTest.cpp
   LIBRARIES casalib cmbapi DataAccess SerialDataAccess TableIO ${HDF5_LIBRARIES} ${Boost_LIBRARIES}
)

add_gtest( NAME DoETest
   SOURCES test/DoETest.cpp
   LIBRARIES casalib cmbapi DataAccess SerialDataAccess TableIO ${HDF5_LIBRARIES} ${Boost_LIBRARIES}
)

add_gtest( NAME MutatorTest
   SOURCES test/MutatorTest.cpp
   LIBRARIES casalib cmbapi DataAccess SerialDataAccess TableIO ${HDF5_LIBRARIES} ${Boost_LIBRARIES}
)

add_gtest( NAME ValidatorTest
   SOURCES test/ValidatorTest.cpp
   LIBRARIES casalib cmbapi DataAccess SerialDataAccess TableIO ${HDF5_LIBRARIES} ${Boost_LIBRARIES}
)

if (LSF_HOME)
   add_gtest( NAME RunManagerTest
      SOURCES test/RunManagerTest.cpp
      INCLUDE_DIRS ${LSF_INCLUDE_DIR}
      LIBRARIES casalib cmbapi DataAccess SerialDataAccess TableIO ${HDF5_LIBRARIES} ${Boost_LIBRARIES} ${LSF_LIBS}
   )
endif()

add_gtest( NAME DataDiggerTest
   SOURCES test/DataDiggerTest.cpp
   LIBRARIES casalib cmbapi DataAccess SerialDataAccess TableIO ${HDF5_LIBRARIES} ${Boost_LIBRARIES}
)

add_gtest( NAME RSProxyTest
   SOURCES test/RSProxyTest.cpp
   LIBRARIES casalib cmbapi DataAccess SerialDataAccess TableIO ${HDF5_LIBRARIES} ${Boost_LIBRARIES}
)

add_gtest( NAME MCTest
   SOURCES test/MCTest.cpp
   LIBRARIES casalib cmbapi DataAccess SerialDataAccess TableIO ${HDF5_LIBRARIES} ${Boost_LIBRARIES}
)

add_gtest( NAME SerializationTest
   SOURCES test/SerializationTest.cpp
   LIBRARIES casalib cmbapi DataAccess SerialDataAccess TableIO ${HDF5_LIBRARIES} ${Boost_LIBRARIES}
)

add_gtest( NAME BestMatchedCaseGenerationTest
   SOURCES test/BestMatchedCaseGenerationTest.cpp
   LIBRARIES casalib cmbapi DataAccess SerialDataAccess TableIO ${HDF5_LIBRARIES} ${Boost_LIBRARIES}
)

add_gtest( NAME SensCalcTest
   SOURCES test/SensCalcTest.cpp
   LIBRARIES casalib cmbapi DataAccess SerialDataAccess TableIO ${HDF5_LIBRARIES} ${Boost_LIBRARIES}
)
# temporary disabled, under development
#add_gtest( NAME LMOptimTest
#   SOURCES test/LMOptimTest.cpp
#   LIBRARIES casalib cmbapi DataAccess SerialDataAccess TableIO ${HDF5_LIBRARIES} ${Boost_LIBRARIES}
#)

if (MSVC)
   generate_version_by_git_last_checkin(src API_FILE_GIT_DATE_AS_VER)
   MESSAGE(STATUS "Version extracted for CasaAPI as: ${API_FILE_GIT_DATE_AS_VER}")
   ##### Generate the C# API  
   generate_csharp_api(
      CSPROJ_NAME                   CasaAPI
      CSPROJ_NAMESPACE              Bpa2.Basin.Casa
      CSPROJ_ASSEMBLY_VERSION       ${API_FILE_GIT_DATE_AS_VER}
      # Info for Nuget package
      CSPROJ_ASSEMBLY_OWNER         "Sergey Koshelev"
      CSPROJ_ASSEMBLY_DESCRIPTION   "C# API for Computer Aided Scenario Analysis"
      CSPROJ_ASSEMBLY_RELEASE_NOTES "Version ${CSPROJ_ASSEMBLY_VERSION}. Initial nuget package release"
      CSPROJ_ASSEMBLY_TAGS          "Cauldron backend CASA SA/UA/SAC Inversion API"
      CSPROJ_ASSEMBLY_COPYRIGHT     "Copyright (C) 2012-2017"
      CSPROJ_LIBRARIES              casalib cmbapi Serial_Hdf5 TableIO DataModel SerialDataAccess DataAccess EosPack ${Boost_LIBRARIES}
      CSHARP_UNIT_TESTS_SRC         test/cmbAPITest.cs test/casaAPITest.cs test/casaAPI_SensitivityCalculatorTest.cs
   )
   
   configure_file(test/Project.project3d ${CMAKE_CURRENT_BINARY_DIR}/Project.project3d COPYONLY ) 
endif (MSVC)
