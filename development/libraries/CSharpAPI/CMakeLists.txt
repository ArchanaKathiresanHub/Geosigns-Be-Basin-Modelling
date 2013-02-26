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
	PROPERTIES SWIG_FLAGS "-Fmicrosoft;-namespace;Shell.BasinModeling.CSharpAPI;-dllimport;CSharpAPI.dll"
	CPLUSPLUS ON
)

swig_add_module( CSharpAPI csharp src/Swig/DotNetAPI.i
	${CMAKE_CURRENT_BINARY_DIR}/version.rc
)

#add_custom_command( TARGET CSharpAPI
#	PRE_BUILD
#   COMMAND del
#	ARGS "/q" "*.cs"
#	WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/csharp"
#)

swig_link_libraries( CSharpAPI Serial_Hdf5 TableIO SerialDataAccess DataAccess EosPack)
#set_source_files_properties( ${swig_generated_file_fullname}
#		PROPERTIES COMPILE_FLAGS "-D_HDF5USEDLL_ -DHDF5CPP_USEDLL"
#)

set_target_properties( CSharpAPI PROPERTIES
	LINK_FLAGS_DEBUG /NODEFAULTLIB:"libcmtd.lib"
	LINK_FLAGS_RELEASE /NODEFAULTLIB:"libcmt.lib"
	)


# I wish I could configure the C# project file in such a way that the names of the generated SWIG files
# are replaced via variable substitution. However, with the current version of CMake this hard to do.
# Therefore, the names of the generated C# files are hard-coded in the .csproj file below.
new_guid(Guid)
configure_file( Shell.BasinModeling.CSharpAPI.csproj.cmake ${CMAKE_CURRENT_BINARY_DIR}/csharp/Shell.BasinModeling.CSharpAPI.csproj @ONLY)
include_external_msproject(Shell.BasinModeling.CSharpAPI ${CMAKE_CURRENT_BINARY_DIR}/csharp/Shell.BasinModeling.CSharpAPI.csproj
	TYPE "{FAE04EC0-301F-11D3-BF4B-00C04F79EFBC}" # This GUID is a Windows C# project (see also http://msdn.microsoft.com/en-us/library/hb23x61k(v=vs.80).aspx ) 
	PLATFORM "${BM_WINDOWS_PLATFORM}"
	GUID "${Guid}"
	CSharpAPI
)
	
###### Installation
set(flavors "${CMAKE_CONFIGURATION_TYPES}")
if (NOT flavors)
    set(flavors "${CMAKE_BUILD_TYPE}")
endif()
	
install(TARGETS CSharpAPI
	RUNTIME DESTINATION bin
	LIBRARY	DESTINATION bin
)
	
foreach(flavor ${flavors})
    install( PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/csharp/${flavor}/Shell.BasinModeling.CSharpAPI.dll
	    DESTINATION	bin
    )
endforeach(flavor)
	
set( CSHARP_UNIT_TESTS_SRC
	test/SerialDataAccessTest.cs
	test/PTDiagCalculatorTest.cs
)
###### Compile and run unit tests
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
