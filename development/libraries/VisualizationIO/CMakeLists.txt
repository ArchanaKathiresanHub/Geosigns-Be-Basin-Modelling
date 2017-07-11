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

generate_version_by_git_last_checkin(src API_FILE_GIT_DATE_AS_VER)
if ( MSVC )
   MESSAGE(STATUS "Version extracted for VisualizationIOAPI as: ${API_FILE_GIT_DATE_AS_VER}")
   ##### Generate the C# API  
   generate_csharp_api(
      CSPROJ_NAME                   VisualizationIOAPI
      CSPROJ_NAMESPACE              Shell.BasinModeling.CauldronIO
      CSPROJ_ASSEMBLY_VERSION       ${API_FILE_GIT_DATE_AS_VER}
      # Info for Nuget package
      CSPROJ_ASSEMBLY_OWNER         "Sebastiaan Kole"
      CSPROJ_ASSEMBLY_DESCRIPTION   "C#/C++ Cauldron API to access simulation results data for vizualization"
      CSPROJ_ASSEMBLY_RELEASE_NOTES "Version ${API_FILE_GIT_DATE_AS_VER}. Initial nuget package release"
      CSPROJ_ASSEMBLY_TAGS          "Cauldron backend vizualization data 4DViewer API"
      CSPROJ_ASSEMBLY_COPYRIGHT     "Copyright (C) 2012-2017"
      CSPROJ_ADDITIONAL_LIBRARIES   VisualizationIO.lib ../FileSystem/FileSystem.lib ../../LZlib/LZlib.lib ../../PugiXMLlib/PugiXMLlib.lib
      CSPROJ_ADDITIONAL_HEADERS     VisualizationAPI.h VisualizationAPIFwDecl.h ImportFromXML.h DataStore.h
                                    ../../FileSystem/src/FilePath.h 
                                    ../../FileSystem/src/Path.h 
                                    ../../FileSystem/src/FolderPath.h 
                                    ../../utilities/src/FormattingException.h
                                    ../../../../3rdparty/lz4/include/lz4.h
                                    ../../../../3rdparty/pugixml-1.7/include/pugiconfig.hpp
                                    ../../../../3rdparty/pugixml-1.7/include/pugixml.hpp
      CSPROJ_LIBRARIES              VisualizationIO DataModel DataAccess utilities SerialDataAccess FileSystem TableIO Serial_Hdf5 CBMGenerics PugiXMLlib LZlib ${HDF5_LIBRARIES} ${Boost_LIBRARIES}
      CSHARP_UNIT_TESTS_SRC         test/ProjectTest.cs test/PVTDataTest.cs
   )

   configure_file(test/data/HydrostaticGL_out.xml            ${CMAKE_CURRENT_BINARY_DIR}/csharp-test/HydrostaticGL_out.xml COPYONLY)
   configure_file(test/data/Snapshot_0.000000_volumes.cldrn  ${CMAKE_CURRENT_BINARY_DIR}/csharp-test/Snapshot_0.000000_volumes.cldrn  COPYONLY)
   configure_file(test/data/Snapshot_5.000000_volumes.cldrn  ${CMAKE_CURRENT_BINARY_DIR}/csharp-test/Snapshot_5.000000_volumes.cldrn  COPYONLY)
   configure_file(test/data/Snapshot_10.000000_volumes.cldrn ${CMAKE_CURRENT_BINARY_DIR}/csharp-test/Snapshot_10.000000_volumes.cldrn COPYONLY)

   configure_file(test/pvtdata/pvtdata.xml                      ${CMAKE_CURRENT_BINARY_DIR}/csharp-test/pvtdata/pvtdata.xml                      COPYONLY)
   configure_file(test/pvtdata/Input_surfaces.cldrn             ${CMAKE_CURRENT_BINARY_DIR}/csharp-test/pvtdata/Input_surfaces.cldrn             COPYONLY)
   configure_file(test/pvtdata/migration_events.cldrn           ${CMAKE_CURRENT_BINARY_DIR}/csharp-test/pvtdata/migration_events.cldrn           COPYONLY)
   configure_file(test/pvtdata/trap_table.cldrn                 ${CMAKE_CURRENT_BINARY_DIR}/csharp-test/pvtdata/trap_table.cldrn                 COPYONLY)
   configure_file(test/pvtdata/trapper_table.cldrn              ${CMAKE_CURRENT_BINARY_DIR}/csharp-test/pvtdata/trapper_table.cldrn              COPYONLY)
   configure_file(test/pvtdata/Snapshot_0.000000_volumes.cldrn  ${CMAKE_CURRENT_BINARY_DIR}/csharp-test/pvtdata/Snapshot_0.000000_volumes.cldrn  COPYONLY)
   configure_file(test/pvtdata/Snapshot_0.000000_surfaces.cldrn ${CMAKE_CURRENT_BINARY_DIR}/csharp-test/pvtdata/Snapshot_0.000000_surfaces.cldrn COPYONLY)

else (MSVC) # UNIX
   if (NUGET_FOUND)
      generate_nuget_pkg( PKG_NAME          VisualizationIOAPI
                          PKG_NAMESPACE     Shell.BasinModeling.CauldronIO
                          PKG_VERSION       ${API_FILE_GIT_DATE_AS_VER}
                          PKG_OWNER         "Sebastiaan.Kole@pds.nl"
                          PKG_DESCRIPTION   "C#/C++ Cauldron API to access simulation results data for vizualization"
                          PKG_RELEASE_NOTES "Version ${API_FILE_GIT_DATE_AS_VER}. Initial nuget package release"
                          PKG_LIBRARIES     VisualizationIO ../FileSystem/FileSystem ../../LZlib/LZlib ../../PugiXMLlib/PugiXMLlib
                          PKG_HEADERS       VisualizationAPI.h VisualizationAPIFwDecl.h ImportFromXML.h DataStore.h
                                            ../../FileSystem/src/FilePath.h 
                                            ../../FileSystem/src/Path.h 
                                            ../../FileSystem/src/FolderPath.h 
                                            ../../utilities/src/FormattingException.h
                                            ../../../../3rdparty/lz4/include/lz4.h
                                            ../../../../3rdparty/pugixml-1.7/include/pugiconfig.hpp
                                            ../../../../3rdparty/pugixml-1.7/include/pugixml.hpp
                        )
   endif (NUGET_FOUND)  
endif (MSVC)

generate_dox( VisualizationIO.cfg )

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
