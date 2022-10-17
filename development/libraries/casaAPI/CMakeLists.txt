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

set (CMAKE_CXX_STANDARD 14)
set (CMAKE_C_STANDARD 14)

set( LIB_NAME "casalib" )

include_directories(
   src
)

include_directories( SYSTEM
   ${SUMPP_INCLUDE_DIR}
   ${CMAKE_CURRENT_BINARY_DIR}
   ${EIGEN_INCLUDE_DIRS}
   ${NNLIB_INCLUDE_DIR}
   ${HDF5_INCLUDE_DIRS}
)

create_bm_library( TARGET ${LIB_NAME}
                   LIBRARIES cmbapi
                             DataAccess
                             EosPack
                             TableIO
                             FileSystem
                             utilities
                             SUMlib
                             modelpseudo1dlib
                             NNlib
                             WorkLoadManagers
                             utilities
                             ${Boost_LIBRARIES} )

if (UNIX)
  add_dependencies(${LIB_NAME} Boost)
endif (UNIX)

configure_file(src/CauldronEnvConfig.h.in CauldronEnvConfig.h)
generate_dox( src/API.cfg )

#######################################
### Unit Tests
#######################################
#Various cauldron projects which are used in unit tests
copy_test_file(Ottoland.project3d)

#Cauldron project with results for DataDigger unit testing
copy_test_file(OttolandWithGenex.project3d)
copy_test_file(OttolandCatPrms.project3d)
copy_test_file(OttolandWithGenexObservables.project3d)
copy_test_file(LithologyTesting.project3d)
configure_file(test/Ottoland_casa_state.txt Ottoland_casa_state.txt)

# Generate best match case test
configure_file(test/BestMatchCaseGeneration_state.txt BestMatchCaseGeneration_state.txt)
copy_test_file(NVG_Project.project3d)
copy_test_file(MAP-72981789-4.FLT)
copy_test_file(Inputs.HDF)

# Files for Ameland case
configure_file(test/Ameland/Ameland.project3d ${CMAKE_CURRENT_BINARY_DIR}/Ameland/Ameland.project3d COPYONLY)

macro(add_casa_gtest)
  cmake_parse_arguments( "CASA" "" "NAME" "SOURCES" ${ARGN} )
  add_gtest( NAME "casaAPI::${CASA_NAME}"
             SOURCES ${CASA_SOURCES}
             LIBRARIES ${LIB_NAME} cmbapi DataAccess SerialDataAccess TableIO ${HDF5_LIBRARIES} ${Boost_LIBRARIES}
             FOLDER "${BASE_FOLDER}/${LIB_NAME}"
           )
endmacro(add_casa_gtest)

add_casa_gtest( NAME BLRSTest SOURCES test/BLRSTest.cpp )

add_casa_gtest( NAME DoETest SOURCES test/DoETest.cpp
)

add_casa_gtest( NAME MutatorTest SOURCES test/MutatorTest.cpp )

add_casa_gtest( NAME ValidatorTest SOURCES test/ValidatorTest.cpp )

add_casa_gtest( NAME DataDiggerTest SOURCES test/DataDiggerTest.cpp )

add_casa_gtest( NAME RSProxyTest SOURCES test/RSProxyTest.cpp )

add_casa_gtest( NAME MCTest SOURCES test/MCTest.cpp )

add_casa_gtest( NAME SerializationTest SOURCES test/SerializationTest.cpp )

add_casa_gtest( NAME BestMatchedCaseGenerationTest SOURCES test/BestMatchedCaseGenerationTest.cpp )

add_casa_gtest( NAME SensCalcTest SOURCES test/SensCalcTest.cpp )

add_casa_gtest( NAME RSProxyQualityCalcTest SOURCES test/RSProxyQualityCalcTest.cpp )

add_casa_gtest( NAME RunCaseSetTest SOURCES test/RunCaseSetTest.cpp)

add_casa_gtest( NAME ObservableTest SOURCES test/ObservableTest.cpp )

add_casa_gtest( NAME RunManagerTest SOURCES test/RunManagerTest.cpp )

add_casa_gtest( NAME PrmEquilibriumOceanicLithosphereThickness   SOURCES test/PrmEquilibriumOceanicLithosphereThickness.cpp )
add_casa_gtest( NAME PrmSurfaceTemperatureTest            SOURCES test/PrmSurfaceTemperatureTest.cpp )
add_casa_gtest( NAME PrmTopCrustHeatProduction            SOURCES test/PrmTopCrustHeatProductionTest.cpp )
add_casa_gtest( NAME PrmTopCrustHeatProductionGrid        SOURCES test/PrmTopCrustHeatProductionGridTest.cpp )
add_casa_gtest( NAME PrmTopCrustHeatProductionGridScaling SOURCES test/PrmTopCrustHeatProductionGridScalingTest.cpp )

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
      CSPROJ_LIBRARIES              ${LIB_NAME} cmbapi Serial_Hdf5 TableIO DataModel SerialDataAccess DataAccess EosPack ${Boost_LIBRARIES}
      CSHARP_UNIT_TESTS_SRC         test/cmbAPITest.cs test/casaAPITest.cs test/casaAPI_SensitivityCalculatorTest.cs
   )

   configure_file(test/Project.project3d ${CMAKE_CURRENT_BINARY_DIR}/Project.project3d COPYONLY )
endif (MSVC)

add_subdirectory(test/sumlibTests)
