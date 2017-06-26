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

FILE( GLOB all_headers src/*.h)
source_group("Header Files" FILES ${all_headers})

FILE( GLOB all_srcs src/*.cpp)
source_group("Source Files" FILES ${all_srcs})

add_library(VisualizationIO ${all_srcs} ${all_headers})

include_directories( SYSTEM
   src
   ${PUGIXML_INCLUDE_DIR}
   ${LZ_INCLUDE_DIR}
)

bm_include_libraries(
   utilities
   FileSystem
)

target_link_libraries(VisualizationIO
   utilities
   FileSystem
   PugiXMLlib
   LZlib
   ${Boost_LIBRARIES}
)

if ( MSVC )
   generate_version_by_git_last_checkin(src API_FILE_GIT_DATE_AS_VER)
   
   MESSAGE(STATUS "Version extracted for VisualizationIOAPI as: ${API_FILE_GIT_DATE_AS_VER}")
   ##### Generate the C# API  
   generate_csharp_api(
      CSPROJ_NAME                   VisualizationIOAPI
      CSPROJ_NAMESPACE              Shell.BasinModeling.CauldronIO
      CSPROJ_ASSEMBLY_VERSION       ${API_FILE_GIT_DATE_AS_VER}
      # Info for Nuget package
      CSPROJ_ASSEMBLY_OWNER         "Sebastiaan Kole"
      CSPROJ_ASSEMBLY_DESCRIPTION   "C# Cauldron API to access simulation results data for vizualization"
      CSPROJ_ASSEMBLY_RELEASE_NOTES "Version ${API_FILE_GIT_DATE_AS_VER}. Initial nuget package release"
      CSPROJ_ASSEMBLY_TAGS          "Cauldron backend vizualization data 4DViewer API"
      CSPROJ_ASSEMBLY_COPYRIGHT     "Copyright (C) 2012-2017"
      CSPROJ_LIBRARIES              VisualizationIO DataModel DataAccess utilities SerialDataAccess FileSystem TableIO Serial_Hdf5 CBMGenerics PugiXMLlib LZlib ${HDF5_LIBRARIES} ${Boost_LIBRARIES}
      CSHARP_UNIT_TESTS_SRC         test/ProjectTest.cs
   )

   configure_file(test/data/HydrostaticGL_out.xml            ${CMAKE_CURRENT_BINARY_DIR}/csharp-test/HydrostaticGL_out.xml COPYONLY)
   configure_file(test/data/Snapshot_0.000000_volumes.cldrn  ${CMAKE_CURRENT_BINARY_DIR}/csharp-test/Snapshot_0.000000_volumes.cldrn  COPYONLY)
   configure_file(test/data/Snapshot_5.000000_volumes.cldrn  ${CMAKE_CURRENT_BINARY_DIR}/csharp-test/Snapshot_5.000000_volumes.cldrn  COPYONLY)
   configure_file(test/data/Snapshot_10.000000_volumes.cldrn ${CMAKE_CURRENT_BINARY_DIR}/csharp-test/Snapshot_10.000000_volumes.cldrn COPYONLY)
endif (MSVC)

generate_dox( VisualizationIO.cfg )

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
