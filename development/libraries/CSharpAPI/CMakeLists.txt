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
   ${SUMPP_INCLUDE_DIR}
   ${HDF5_INCLUDE_DIRS}
)

bm_include_libraries( 
   DataModel
   CBMGenerics
   Serial_Hdf5
   TableIO
   DataAccess
   cmbAPI
   casaAPI
   utilities
)

set(CMAKE_SWIG_OUTDIR ${CMAKE_CURRENT_BINARY_DIR}/csharp)

# Note to the following line:
#    Use quotes to set the value of SWIG_FLAGS. The semi-colon makes sure 
#    that the value becomes space separated again, when the build script 
#    is generated
set_source_files_properties( src/Swig/DotNetAPI.i
    PROPERTIES SWIG_FLAGS "-namespace;Shell.BasinModeling.Cauldron;-dllimport;Cauldron.dll"
    CPLUSPLUS ON
)

swig_add_module( Cauldron csharp src/Swig/DotNetAPI.i
    ${CMAKE_CURRENT_BINARY_DIR}/version.rc
)

swig_link_libraries( Cauldron casaapi cmbapi Serial_Hdf5 TableIO SerialDataAccess DataAccess EosPack ${Boost_LIBRARIES} )

# Before C# generation, remove all existing files. The directory should be empty before generation	
add_custom_command( TARGET Cauldron
   PRE_BUILD
   COMMAND ${CMAKE_COMMAND} ARGS "-E" "remove" "*.cs"
   WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/csharp"
)

# Visual studio should reload the project after C# generation
add_custom_command(TARGET Cauldron
    PRE_LINK 
    COMMAND ${CMAKE_COMMAND} ARGS "-E" "touch_nocreate" "Shell.BasinModeling.Cauldron.csproj"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/csharp"
)

# Generate C# project to compile generated C# files. 
new_guid(Guid)
configure_file( Shell.BasinModeling.Cauldron.csproj.cmake ${CMAKE_CURRENT_BINARY_DIR}/csharp/read-only/Shell.BasinModeling.Cauldron.csproj @ONLY)
# TFS checksout files in 'Read only' mode. configure_file retains this mode, however we want to 'touch' the file when the C# files have been generated
# So copy it, with the right permissions.
file(COPY ${CMAKE_CURRENT_BINARY_DIR}/csharp/read-only/Shell.BasinModeling.Cauldron.csproj
  DESTINATION  ${CMAKE_CURRENT_BINARY_DIR}/csharp
  FILE_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ 
)
include_external_msproject(Shell.BasinModeling.Cauldron ${CMAKE_CURRENT_BINARY_DIR}/csharp/Shell.BasinModeling.Cauldron.csproj
    TYPE "{FAE04EC0-301F-11D3-BF4B-00C04F79EFBC}" # This GUID is a Windows C# project (see also http://msdn.microsoft.com/en-us/library/hb23x61k(v=vs.80).aspx ) 
    PLATFORM "${BM_WINDOWS_PLATFORM}"
    GUID "${Guid}"
    Cauldron
)
    
###### Installation
install(TARGETS Cauldron
    RUNTIME DESTINATION bin
    LIBRARY	DESTINATION bin
)
    
install( PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/csharp/Debug/Shell.BasinModeling.Cauldron.dll
   DESTINATION	bin
   CONFIGURATIONS Debug
)

install( PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/csharp/Release/Shell.BasinModeling.Cauldron.dll
   DESTINATION	bin
   CONFIGURATIONS Release
)


###### Compile and run unit tests	
set( CSHARP_UNIT_TESTS_SRC
    test/SerialDataAccessTest.cs
    test/PTDiagCalculatorTest.cs
    test/EospackCApiTest.cs
    test/BpaComponent.cs
    test/BpaPhase.cs
    test/cmbAPITest.cs
    test/casaAPITest.cs
)

configure_file(test/AssemblyInfo.cs.cmake csharp-test/Properties/AssemblyInfo.cs)
configure_file(test/Project.project3d csharp-test/Project.project3d)
configure_file(test/Ottoland.project3d csharp-test/Ottoland.project3d)
configure_file(test/PTDiagCalculatorTest.cfg csharp-test/PTDiagCalculatorTest.cfg)

add_csharp_unittest( 
  NAME Shell.BasinModeling.Cauldron.Test
  PLATFORM ${BM_WINDOWS_PLATFORM}
  DIRECTORY csharp-test
  TEST_SOURCES ${CSHARP_UNIT_TESTS_SRC}
  DEPENDS Shell.BasinModeling.Cauldron
  TESTLIST "Test"
  TFS_SERVER_URL ${BM_TFS_SERVER_URL}
  TFS_BUILD_NUMBER ${BM_TFS_BUILD_NUMBER}
  TFS_PROJECT_NAME ${BM_TFS_PROJECT_NAME}
  DEPLOYMENT_ITEMS 
    "Debug/Cauldron.dll" 
    "Release/Cauldron.dll" 
  PROJECT_REFERENCE
    Shell.BasinModeling.Cauldron
    "${Guid}"
    "${CMAKE_CURRENT_BINARY_DIR}/csharp/Shell.BasinModeling.Cauldron.csproj"
)
