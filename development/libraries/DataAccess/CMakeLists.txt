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

FILE( GLOB all_headers src/*.h src/Interface/*.h )
source_group(include FILES ${all_headers})

FILE( GLOB all_srcs src/*.C src/*.cpp)
source_group(source FILES ${all_srcs})

set( LIB_NAME "DataAccess" )
add_library(${LIB_NAME} 
   ${all_srcs}
   ${all_headers}
)
set_target_properties( ${LIB_NAME} PROPERTIES FOLDER "${BASE_FOLDER}/${LIB_NAME}" )

bm_include_libraries(
   DataModel
   Serial_Hdf5
   CBMGenerics
   utilities
   FileSystem 
   TableIO
   EosPack
)

include_directories( src )

include_directories( SYSTEM
   ${HDF5_INCLUDE_DIRS}
)

target_link_libraries(${LIB_NAME} 
   ${Boost_LIBRARIES}
   Serial_Hdf5
   CBMGenerics
   FileSystem
   utilities
   TableIO
)

if (UNIX)
   add_dependencies(${LIB_NAME} HDF5)
endif (UNIX)

add_gtest( NAME ${LIB_NAME}
           SOURCES test/DataAccess.C
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../SerialDataAccess/src/Interface/
           LIBRARIES ${LIB_NAME} SerialDataAccess Serial_Hdf5
           ENV_VARS EOSPACKDIR=${CFGFLS}/eospack GENEX5DIR=${CFGFLS}/genex50 CTCDIR=${CFGFLS}/
           FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

configure_file(
   ${CMAKE_CURRENT_SOURCE_DIR}/test/DataAccessTest.project3d
   DataAccessTest.project3d
   COPYONLY
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

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
