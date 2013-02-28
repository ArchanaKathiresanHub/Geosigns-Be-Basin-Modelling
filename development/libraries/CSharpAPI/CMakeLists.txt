##### Generate the C# API
configure_file(src/version.rc.cmake version.rc)
configure_file(src/AssemblyInfo.cs.cmake csharp/Properties/AssemblyInfo.cs)

if (NOT MSVC)
  message(FATAL_ERROR "MS Visual studio is necessary to build the C sharp interface")
endif()

include_directories(
   src/Swig
   ${HDF5_serial_INCLUDE_DIRS}
)

bm_include_libraries( 
   CBMGenerics
   Serial_Hdf5
   TableIO
   DataAccess
)

set(CMAKE_SWIG_OUTDIR ${CMAKE_CURRENT_BINARY_DIR}/csharp)

# Note to the following line:
#    Use quotes to set the value of SWIG_FLAGS. The semi-colon makes sure 
#    that the value becomes space separated again, when the build script 
#    is generated
set_source_files_properties( src/Swig/DotNetAPI.i
	PROPERTIES SWIG_FLAGS "-namespace;Shell.BasinModeling.CSharpAPI;-dllimport;CSharpAPI.dll"
	CPLUSPLUS ON
)

swig_add_module( CSharpAPI csharp src/Swig/DotNetAPI.i
	${CMAKE_CURRENT_BINARY_DIR}/version.rc
)

swig_link_libraries( CSharpAPI Serial_Hdf5 TableIO SerialDataAccess DataAccess EosPack)

# Before C# generation, remove all existing files. The directory should be empty before generation	
add_custom_command( TARGET CSharpAPI
   PRE_BUILD
   COMMAND ${CMAKE_COMMAND} ARGS "-E" "remove" "*.cs"
   WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/csharp"
)

# Visual studio should reload the project after C# generation
add_custom_command(TARGET CSharpAPI
	PRE_LINK 
	COMMAND ${CMAKE_COMMAND} ARGS "-E" "touch_nocreate" "Shell.BasinModeling.CSharpAPI.csproj"
	WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/csharp"
)

# Generate C# project to compile generated C# files. 
new_guid(Guid)
configure_file( Shell.BasinModeling.CSharpAPI.csproj.cmake ${CMAKE_CURRENT_BINARY_DIR}/csharp/read-only/Shell.BasinModeling.CSharpAPI.csproj @ONLY)
# TFS checksout files in 'Read only' mode. configure_file retains this mode, however we want to 'touch' the file when the C# files have been generated
# So copy it, with the right permissions.
file(COPY ${CMAKE_CURRENT_BINARY_DIR}/csharp/read-only/Shell.BasinModeling.CSharpAPI.csproj
  DESTINATION  ${CMAKE_CURRENT_BINARY_DIR}/csharp
  FILE_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ 
)
include_external_msproject(Shell.BasinModeling.CSharpAPI ${CMAKE_CURRENT_BINARY_DIR}/csharp/Shell.BasinModeling.CSharpAPI.csproj
	TYPE "{FAE04EC0-301F-11D3-BF4B-00C04F79EFBC}" # This GUID is a Windows C# project (see also http://msdn.microsoft.com/en-us/library/hb23x61k(v=vs.80).aspx ) 
	PLATFORM "${BM_WINDOWS_PLATFORM}"
	GUID "${Guid}"
	CSharpAPI
)
	
###### Installation
install(TARGETS CSharpAPI
	RUNTIME DESTINATION bin
	LIBRARY	DESTINATION bin
)
	
install( PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/csharp/Debug/Shell.BasinModeling.CSharpAPI.dll
   DESTINATION	bin
   CONFIGURATIONS Debug
)

install( PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/csharp/Release/Shell.BasinModeling.CSharpAPI.dll
   DESTINATION	bin
   CONFIGURATIONS Release
)


###### Compile and run unit tests	
set( CSHARP_UNIT_TESTS_SRC
	test/SerialDataAccessTest.cs
	test/PTDiagCalculatorTest.cs
)

configure_file(test/AssemblyInfo.cs.cmake csharp-test/Properties/AssemblyInfo.cs)
configure_file(test/Project.project3d csharp-test/Project.project3d)
configure_file(test/PTDiagCalculatorTest.cfg csharp-test/PTDiagCalculatorTest.cfg)

add_csharp_unittest( 
  NAME Shell.BasinModeling.CSharpAPI.Test
  PLATFORM ${BM_WINDOWS_PLATFORM}
  DIRECTORY csharp-test
  TEST_SOURCES test/SerialDataAccessTest.cs test/PTDiagCalculatorTest.cs
  DEPENDS Shell.BasinModeling.CSharpAPI
  TESTLIST "Test"
  TFS_SERVER_URL ${BM_TFS_SERVER_URL}
  TFS_BUILD_NUMBER ${BM_TFS_BUILD_NUMBER}
  TFS_PROJECT_NAME ${BM_TFS_PROJECT_NAME}
  DEPLOYMENT_ITEMS 
	"Debug/CSharpAPI.dll" 
	"Release/CSharpAPI.dll" 
  PROJECT_REFERENCE
    Shell.BasinModeling.CSharpAPI
	"${Guid}"
	"${CMAKE_CURRENT_BINARY_DIR}/csharp/Shell.BasinModeling.CSharpAPI.csproj"
)
