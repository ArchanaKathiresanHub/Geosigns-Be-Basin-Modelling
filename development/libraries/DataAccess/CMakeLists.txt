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

set( LIB_NAME "DataAccess" )

include_directories( SYSTEM
   ${HDF5_INCLUDE_DIRS}
)

create_bm_library( TARGET ${LIB_NAME}
                   LIBRARIES
   ${Boost_LIBRARIES}
   ${HDF5_LIBRARIES}
   Serial_Hdf5
   DataModel   
   TableIO
   EosPack
   CBMGenerics
   FileSystem
   utilities
)

add_gtest( NAME DA::DataAccess
           SOURCES test/DataAccess.cpp
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../SerialDataAccess/src/Interface/
           LIBRARIES ${LIB_NAME} SerialDataAccess Serial_Hdf5
           ENV_VARS EOSPACKDIR=${CFGFLS}/eospack GENEX5DIR=${CFGFLS}/genex50 CTCDIR=${CFGFLS}/
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)
copy_test_file(DataAccessTest.project3d)

add_gtest( NAME DA::GenerateSnapshots
           SOURCES test/GenerateSnapshots.cpp
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../SerialDataAccess/src/Interface/
           LIBRARIES ${LIB_NAME} SerialDataAccess Serial_Hdf5
           ENV_VARS EOSPACKDIR=${CFGFLS}/eospack GENEX5DIR=${CFGFLS}/genex50 CTCDIR=${CFGFLS}/
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)
copy_test_file(GenerateSnapshotsTest.project3d)

add_gtest( NAME DA::CharismaFaultFileReaderTest
  SOURCES test/CharismaFaultFileReaderTest.cpp
  INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../SerialDataAccess/src/Interface/
  LIBRARIES ${LIB_NAME} SerialDataAccess FileSystem
  FOLDER "${BASE_FOLDER}/${TARGET_NAME}"
)

copy_test_file(Project.project3d)
copy_test_file(FaultFileTest.charisma)
copy_test_file(FaultEmptyFileTest.charisma)
copy_test_file(FaultIntersect.charisma)
copy_test_file(FaultNoIntersect.charisma)
copy_test_file(testDomainShapeReader.h5)

add_gtest( NAME DA::TriangleTest
  SOURCES test/TriangleTest.cpp
  INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../SerialDataAccess/src/Interface/
  LIBRARIES ${LIB_NAME} SerialDataAccess
  FOLDER "${BASE_FOLDER}/${TARGET_NAME}"
)

add_gtest( NAME DA::FaultPlaneTest
  SOURCES test/FaultPlaneTest.cpp
  INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../SerialDataAccess/src/Interface/
  LIBRARIES ${LIB_NAME} SerialDataAccess
  FOLDER "${BASE_FOLDER}/${TARGET_NAME}"
)

add_gtest( NAME DA::FaultCutCreatorTest
  SOURCES test/faultcutcreatortest.cpp
  INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../SerialDataAccess/src/Interface/
  LIBRARIES ${LIB_NAME} SerialDataAccess
  FOLDER "${BASE_FOLDER}/${TARGET_NAME}"
)

add_gtest( NAME DA::FaultFileReaderFactoryTest
  SOURCES test/FaultFileReaderFactoryTest.cpp
  INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../SerialDataAccess/src/Interface/
  LIBRARIES ${LIB_NAME} SerialDataAccess FileSystem
  FOLDER "${BASE_FOLDER}/${TARGET_NAME}"
)

add_gtest( NAME DA::DomainShapeReaderTest
  SOURCES test/domainShapeReaderTest.cpp
  INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../SerialDataAccess/src/Interface/
  LIBRARIES ${LIB_NAME} SerialDataAccess
  FOLDER "${BASE_FOLDER}/${TARGET_NAME}"
)

generate_dox( src/DataAccess.cfg )

if ( MSVC )
   generate_version_by_git_last_checkin(src/Interface API_FILE_GIT_DATE_AS_VER)

   MESSAGE(STATUS "Version extracted for DataAccessAPI as: ${API_FILE_GIT_DATE_AS_VER}")
   ##### Generate the C# API  
   generate_csharp_api(
      CSPROJ_NAME                   DataAccessAPI
      CSPROJ_NAMESPACE              Bpa2.Basin.DataAccess
      CSPROJ_ASSEMBLY_VERSION       ${API_FILE_GIT_DATE_AS_VER}
      # Info for Nuget package
      CSPROJ_ASSEMBLY_OWNER         "Sergey Koshelev"
      CSPROJ_ASSEMBLY_DESCRIPTION   "C# API for Cauldron input/simulation results data access"
      CSPROJ_ASSEMBLY_RELEASE_NOTES "Version ${API_FILE_GIT_DATE_AS_VER}. Initial nuget package release"
      CSPROJ_ASSEMBLY_TAGS          "Cauldron backend data HDF API"
      CSPROJ_ASSEMBLY_COPYRIGHT     "Copyright (C) 2012-2017"
      CSPROJ_LIBRARIES              Serial_Hdf5 TableIO DataModel SerialDataAccess ${LIB_NAME} EosPack ${Boost_LIBRARIES}
      CSHARP_UNIT_TESTS_SRC         test/SerialDataAccessTest.cs
   )

   configure_file(test/Project.project3d  ${CMAKE_BINARY_DIR}/Project.project3d COPYONLY)
endif (MSVC)
