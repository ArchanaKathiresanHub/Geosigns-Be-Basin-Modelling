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

##### Generate the C# API
configure_file(src/version.rc.cmake version.rc)
configure_file(src/AssemblyInfo.cs.cmake csharp/Properties/AssemblyInfo.cs)

if (NOT MSVC)
  message(FATAL_ERROR "MS Visual studio is necessary to build the C sharp interface")
endif()

include_directories(
   src/Swig
)

include_directories( SYSTEM
   ${HDF5_INCLUDE_DIRS}
)

bm_include_libraries( 
   FileSystem
   utilities
   VisualizationIO
)

set(CMAKE_SWIG_OUTDIR ${CMAKE_CURRENT_BINARY_DIR}/csharp)

set_source_files_properties( src/Swig/VisualizationAPI.i
    PROPERTIES SWIG_FLAGS "-namespace;Shell.BasinModeling.CauldronIO"
	CPLUSPLUS ON
)

swig_add_module( CauldronIO csharp src/Swig/VisualizationAPI.i
    ${CMAKE_CURRENT_BINARY_DIR}/version.rc
)

swig_link_libraries( CauldronIO VisualizationIO DataModel DataAccess utilities SerialDataAccess FileSystem TableIO Serial_Hdf5 CBMGenerics ${HDF5_LIBRARIES} ${Boost_LIBRARIES} )

# Before C# generation, remove all existing files. The directory should be empty before generation	
add_custom_command( TARGET CauldronIO
   PRE_BUILD
   COMMAND ${CMAKE_COMMAND} ARGS "-E" "remove" "*.cs"
   WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/csharp"
)

# Visual studio should reload the project after C# generation
add_custom_command(TARGET CauldronIO
    PRE_LINK 
    COMMAND ${CMAKE_COMMAND} ARGS "-E" "touch_nocreate" "Shell.BasinModeling.CauldronIO.csproj"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/csharp"
)

# Generate C# project to compile generated C# files. 
new_guid(Guid)
configure_file( Shell.BasinModeling.CauldronIO.csproj.cmake ${CMAKE_CURRENT_BINARY_DIR}/csharp/read-only/Shell.BasinModeling.CauldronIO.csproj @ONLY)
# TFS checksout files in 'Read only' mode. configure_file retains this mode, however we want to 'touch' the file when the C# files have been generated
# So copy it, with the right permissions.
file(COPY ${CMAKE_CURRENT_BINARY_DIR}/csharp/read-only/Shell.BasinModeling.CauldronIO.csproj
  DESTINATION  ${CMAKE_CURRENT_BINARY_DIR}/csharp
  FILE_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ 
)

include_external_msproject(Shell.BasinModeling.CauldronIO ${CMAKE_CURRENT_BINARY_DIR}/csharp/Shell.BasinModeling.CauldronIO.csproj
    TYPE "{FAE04EC0-301F-11D3-BF4B-00C04F79EFBC}" # This GUID is a Windows C# project (see also http://msdn.microsoft.com/en-us/library/hb23x61k(v=vs.80).aspx ) 
    PLATFORM "${BM_WINDOWS_PLATFORM}"
    GUID "${Guid}"
    CauldronIO
)
    
###### Installation
install(TARGETS CauldronIO
    RUNTIME DESTINATION bin
    LIBRARY	DESTINATION bin
)
    
install( PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/csharp/Debug/Shell.BasinModeling.CauldronIO.dll
   DESTINATION	bin
   CONFIGURATIONS Debug
)

install( PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/csharp/Release/Shell.BasinModeling.CauldronIO.dll
   DESTINATION	bin
   CONFIGURATIONS Release
)

###### Compile and run unit tests	
set( CSHARP_UNIT_TESTS_SRC
    test/ProjectTest.cs
)

configure_file(test/AssemblyInfo.cs.cmake csharp-test/Properties/AssemblyInfo.cs)

# copy some unit test data
configure_file(test/data/HydrostaticGL_out.xml            ${CMAKE_CURRENT_BINARY_DIR}/csharp-test/HydrostaticGL_out.xml COPYONLY)
configure_file(test/data/Snapshot_0.000000_volumes.cldrn  ${CMAKE_CURRENT_BINARY_DIR}/csharp-test/Snapshot_0.000000_volumes.cldrn  COPYONLY)
configure_file(test/data/Snapshot_5.000000_volumes.cldrn  ${CMAKE_CURRENT_BINARY_DIR}/csharp-test/Snapshot_5.000000_volumes.cldrn  COPYONLY)
configure_file(test/data/Snapshot_10.000000_volumes.cldrn ${CMAKE_CURRENT_BINARY_DIR}/csharp-test/Snapshot_10.000000_volumes.cldrn COPYONLY)

add_csharp_unittest( 
  NAME Shell.BasinModeling.CauldronIO.Test
  PLATFORM ${BM_WINDOWS_PLATFORM}
  DIRECTORY csharp-test
  TEST_SOURCES ${CSHARP_UNIT_TESTS_SRC}
  DEPENDS Shell.BasinModeling.CauldronIO
  TESTLIST "Test"
  TFS_SERVER_URL ${BM_TFS_SERVER_URL}
  TFS_BUILD_NUMBER ${BM_TFS_BUILD_NUMBER}
  TFS_PROJECT_NAME ${BM_TFS_PROJECT_NAME}
  DEPLOYMENT_ITEMS 
    "../\$(Configuration)/CauldronIO.dll" 
  PROJECT_REFERENCE
    Shell.BasinModeling.CauldronIO
    "${Guid}"
    "${CMAKE_CURRENT_BINARY_DIR}/csharp/Shell.BasinModeling.CauldronIO.csproj"
)

