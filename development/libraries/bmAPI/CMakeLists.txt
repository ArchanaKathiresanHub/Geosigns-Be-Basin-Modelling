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

# Building the Project3dAPI library
#
# ATTENTION: The file is used by Basin Modeling and Geocase.
#
include_directories(src)

set (generator_source_files
  ${CMAKE_CURRENT_BINARY_DIR}/cauldronschemaAPI.h
  ${CMAKE_CURRENT_BINARY_DIR}/cauldronschemaAPI.cpp
)

add_library(bmAPI
	${generator_source_files}
)

# under Linux we are building Boost that is why this target exist
if (UNIX)
add_dependencies(bmAPI Boost)
endif (UNIX)

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR}/../TableIO )

add_dependencies(bmAPI
   TableIO
   FileSystem
   utilities
)

bm_include_libraries( bmAPI
   TableIO
   FileSystem
   utilities
)

target_link_libraries( bmAPI
   TableIO
   FileSystem
   utilities
)

add_custom_command( OUTPUT ${generator_source_files}
	DEPENDS parspec ../TableIO/BasinModellerProperty.Spec
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	COMMAND parspec -schema CauldronSchema -directory ${CMAKE_CURRENT_BINARY_DIR} -input ../TableIO/BasinModellerProperty.Spec -bmapi
)

install(TARGETS bmAPI
	RUNTIME DESTINATION bin
	LIBRARY	DESTINATION lib
	ARCHIVE DESTINATION lib
)

configure_file(
      ${CMAKE_CURRENT_SOURCE_DIR}/test/Reference.project3d
      ${CMAKE_CURRENT_BINARY_DIR}
      COPYONLY
)

add_gtest( NAME TableIO::CauldronSchemaAPITest
           SOURCES test/CauldronSchemaAPITest.cpp
           LIBRARIES bmAPI TableIO utilities FileSystem
)

##### Generate the C# API
configure_file(src/version.rc.cmake version.rc)
configure_file(src/AssemblyInfo.cs.cmake csharp/Properties/AssemblyInfo.cs)

if (MSVC)
   include_directories(
      src/Swig
      ${CMAKE_CURRENT_BINARY_DIR}
   )
   
   set(CMAKE_SWIG_OUTDIR ${CMAKE_CURRENT_BINARY_DIR}/csharp)

   set_source_files_properties( src/Swig/Project3dAPI.i
      PROPERTIES SWIG_FLAGS "-namespace;Shell.BasinModeling.Project3dAPI;-I${CMAKE_CURRENT_BINARY_DIR}"
      CPLUSPLUS ON
   )

   swig_add_module(Project3dAPI csharp src/Swig/Project3dAPI.i
      ${CMAKE_CURRENT_BINARY_DIR}/version.rc
   )

   swig_link_libraries( Project3dAPI bmAPI TableIO FileSystem ${Boost_LIBRARIES} )

   # Before C# generation, remove all existing files. The directory should be empty before generation	
   add_custom_command( TARGET Project3dAPI
      PRE_BUILD
      COMMAND ${CMAKE_COMMAND} ARGS "-E" "remove" "*.cs"
      WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/csharp"
   )

   # Visual studio should reload the project after C# generation
   add_custom_command(TARGET Project3dAPI
      PRE_LINK 
      COMMAND ${CMAKE_COMMAND} ARGS "-E" "touch_nocreate" "Shell.BasinModeling.Project3dAPI.csproj"
      WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/csharp"
   )

   # Generate C# project to compile generated C# files. 
   new_guid(Guid)
   configure_file( Shell.BasinModeling.Project3dAPI.csproj.cmake ${CMAKE_CURRENT_BINARY_DIR}/csharp/read-only/Shell.BasinModeling.Project3dAPI.csproj @ONLY)
   # TFS checksout files in 'Read only' mode. configure_file retains this mode, however we want to 'touch' the file when the C# files have been generated
   # So copy it, with the right permissions.
   file(COPY ${CMAKE_CURRENT_BINARY_DIR}/csharp/read-only/Shell.BasinModeling.Project3dAPI.csproj
      DESTINATION  ${CMAKE_CURRENT_BINARY_DIR}/csharp
      FILE_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ
   )

   include_external_msproject(Shell.BasinModeling.Project3dAPI ${CMAKE_CURRENT_BINARY_DIR}/csharp/Shell.BasinModeling.Project3dAPI.csproj
      TYPE "{FAE04EC0-301F-11D3-BF4B-00C04F79EFBC}" # This GUID is a Windows C# project (see also http://msdn.microsoft.com/en-us/library/hb23x61k(v=vs.80).aspx ) 
      PLATFORM "${BM_WINDOWS_PLATFORM}"
      GUID "${Guid}"
      Project3dAPI
   )
    
   ###### Installation
   install(TARGETS Project3dAPI
      RUNTIME DESTINATION bin
      LIBRARY	DESTINATION bin
   )
    
   install( PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/csharp/Debug/Shell.BasinModeling.Project3dAPI.dll
      DESTINATION	bin
      CONFIGURATIONS Debug
   )

   install( PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/csharp/Release/Shell.BasinModeling.Project3dAPI.dll
      DESTINATION	bin
      CONFIGURATIONS Release
   )

   ###### Compile and run unit tests	
   set( CSHARP_UNIT_TESTS_SRC
      test/Project3dAPITest.cs test/Project3dAPITest_gen.cs
   )

   configure_file(test/AssemblyInfo.cs.cmake csharp-test/Properties/AssemblyInfo.cs)

   add_csharp_unittest( NAME Shell.BasinModeling.Project3dAPI.Test
      PLATFORM ${BM_WINDOWS_PLATFORM}
      DIRECTORY csharp-test
      TEST_SOURCES ${CSHARP_UNIT_TESTS_SRC}
      DEPENDS Shell.BasinModeling.Project3dAPI
      TESTLIST "Test"
      TFS_SERVER_URL ${BM_TFS_SERVER_URL}
      TFS_BUILD_NUMBER ${BM_TFS_BUILD_NUMBER}
      TFS_PROJECT_NAME ${BM_TFS_PROJECT_NAME}
      DEPLOYMENT_ITEMS "../\$(Configuration)/Project3dAPI.dll" 
      PROJECT_REFERENCE Shell.BasinModeling.Project3dAPI
      "${Guid}"
      "${CMAKE_CURRENT_BINARY_DIR}/csharp/Shell.BasinModeling.Project3dAPI.csproj"
   )
   
endif()
