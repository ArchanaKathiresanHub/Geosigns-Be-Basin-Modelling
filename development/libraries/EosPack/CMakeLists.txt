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

set( LIB_NAME "EosPack" )
create_bm_library( TARGET ${LIB_NAME}
                   LIBRARIES CBMGenerics
                             utilities )

generate_dox( src/EosPack.cfg )

#######################################
### Unit Tests
#######################################

#######################################
# Unit tests for PTDiagramCalculator

add_gtest( NAME PTDiagramCalculator
   SOURCES test/PTDiagramBuilderTest.cpp test/PVTCfgFileMgr.cpp
   LIBRARIES ${LIB_NAME}
   FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

######################################
# Unit test for PTDiagramBuilderTest in multithreaded environment. 
# Windows run sometimes crashed in Debug version for no obvious reason
if(UNIX)
add_gtest( NAME PTDiagramCalculator_MT
   SOURCES test/PTDiagramBuilderMTTest.cpp test/PVTCfgFileMgr.cpp
   LIBRARIES ${LIB_NAME}
   COMPILE_FLAGS "${OpenMP_CXX_FLAGS}"
   LINK_FLAGS "${OpenMP_CXX_FLAGS} ${OpenMP_LINK_FLAGS}"
   FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)
endif(UNIX)

######################################
# Unit test for flasher
add_gtest( NAME pvtFlash::CAPI
   SOURCES test/EosPackTest.cpp test/PVTCfgFileMgr.cpp
   LIBRARIES ${LIB_NAME}
   FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

######################################
# Unit test for flasher in multithreaded environment
# Windows run sometimes crashed in Debug version for no obvious reason
if(UNIX)
add_gtest( NAME pvtFlash::CAPI_MT
   SOURCES test/EosPackMTTest.cpp test/PVTCfgFileMgr.cpp
   LIBRARIES ${LIB_NAME}
   COMPILE_FLAGS "${OpenMP_CXX_FLAGS}"
   LINK_FLAGS "${OpenMP_CXX_FLAGS} ${OpenMP_LINK_FLAGS}"
   FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)
endif(UNIX)

if (MSVC)
   generate_version_by_git_last_checkin(src API_FILE_GIT_DATE_AS_VER)
   
   MESSAGE(STATUS "Version extracted for EosPackAPI as: ${API_FILE_GIT_DATE_AS_VER}")
   ##### Generate the C# API  
   generate_csharp_api(
      CSPROJ_NAME                   EosPackAPI
      CSPROJ_NAMESPACE              Bpa2.Basin.EOS
      CSPROJ_ASSEMBLY_VERSION       ${API_FILE_GIT_DATE_AS_VER}
      # Info for Nuget package
      CSPROJ_ASSEMBLY_OWNER         "Sergey Koshelev"
      CSPROJ_ASSEMBLY_DESCRIPTION   "C# API for PVT flash and phase diagram calculator"
      CSPROJ_ASSEMBLY_RELEASE_NOTES "Version ${API_FILE_GIT_DATE_AS_VER}. Initial nuget package release"
      CSPROJ_ASSEMBLY_TAGS          "Cauldron backend PVT Flasher PhaseDiagram API"
      CSPROJ_ASSEMBLY_COPYRIGHT     "Copyright (C) 2012-2017"
      CSPROJ_LIBRARIES              ${LIB_NAME}
      CSHARP_UNIT_TESTS_SRC         test/PTDiagCalculatorTest.cs test/EospackCApiTest.cs
   )  
   configure_file(test/PTDiagCalculatorTest.cfg csharp-test/PTDiagCalculatorTest.cfg COPYONLY) 
endif (MSVC)
