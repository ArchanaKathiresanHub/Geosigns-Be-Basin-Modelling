set(SDA_SOURCES
	src/SerialApplicationGlobalOperations.C
	src/SerialGrid.C
	src/SerialGridMap.C
	src/SerialMapWriter.C
	src/SerialMessageHandler.C
	src/SerialObjectFactory.C
	src/SerialProjectHandle.C
	src/Serialhdf5funcs.C
	src/version.rc.cmake
	src/AssemblyInfo.cs.cmake
)

set(SDA_LIBS Serial_Hdf5 CBMGenerics utilities TableIO DataAccess)

bm_include_libraries(
	Serial_Hdf5
	utilities
	TableIO
	CBMGenerics
	DataAccess
)

include_directories(
	${HDF5_serial_INCLUDE_DIRS}
)

generate_dox( src/SerialDataAccess.cfg )

if (BM_CSHARP_API)
	##### Generate the C# API
	
	configure_file(src/version.rc.cmake version.rc)
	configure_file(src/AssemblyInfo.cs.cmake csharp/Properties/AssemblyInfo.cs)
	
	if (NOT MSVC)
	  message(FATAL_ERROR "MS Visual studio is necessary to build the C sharp interface")
	endif()

	include_directories(src/Swig)
	
	set(CMAKE_SWIG_OUTDIR ${CMAKE_CURRENT_BINARY_DIR}/csharp)
	# Note to the following line:
	#    Use quotes to set the value of SWIG_FLAGS. The semi-colon makes sure 
	#    that the value becomes space separated again, when the build script 
	#    is generated
	set_source_files_properties( src/Swig/DotNetAPI.i
		PROPERTIES SWIG_FLAGS "-namespace;Shell.BasinModeling.DataAccess"  
		CPLUSPLUS ON
	)
	swig_add_module( SerialDataAccess csharp src/Swig/DotNetAPI.i 
		${SDA_SOURCES} 
		${CMAKE_CURRENT_BINARY_DIR}/version.rc
	)
	swig_link_libraries( SerialDataAccess Serial_Hdf5 DataAccess )
	#set_source_files_properties( ${swig_generated_file_fullname}
	#		PROPERTIES COMPILE_FLAGS "-D_HDF5USEDLL_ -DHDF5CPP_USEDLL"
	#)

	set_target_properties(SerialDataAccess PROPERTIES
		LINK_FLAGS_DEBUG /NODEFAULTLIB:"libcmtd.lib"
		LINK_FLAGS_RELEASE /NODEFAULTLIB:"libcmt.lib"
		)

		
	# I wish I could configure the C# project file in such a way that the names of the generated SWIG files
	# are replaced via variable substitution. However, with the current version of CMake this hard to do.
	# Therefore, the names of the generated C# files are hard-coded in the .csproj file below, and the file
	# only needs to be copied.
	configure_file( Shell.BasinModeling.DataAccess.csproj.cmake ${CMAKE_CURRENT_BINARY_DIR}/csharp/Shell.BasinModeling.DataAccess.csproj @ONLY)
	
	include_external_msproject(Shell.BasinModeling.DataAccess ${CMAKE_CURRENT_BINARY_DIR}/csharp/Shell.BasinModeling.DataAccess.csproj
		TYPE "{FAE04EC0-301F-11D3-BF4B-00C04F79EFBC}" # This GUID is a Windows C# project (see also http://msdn.microsoft.com/en-us/library/hb23x61k(v=vs.80).aspx ) 
		PLATFORM "${BM_WINDOWS_PLATFORM}"
		SerialDataAccess
	)
		
	###### Installation
	set(flavors "${CMAKE_CONFIGURATION_TYPES}")
	if (NOT flavors)
	    set(flavors "${CMAKE_BUILD_TYPE}")
	endif()
		
	install(TARGETS SerialDataAccess
		RUNTIME DESTINATION bin
		LIBRARY	DESTINATION bin
	)
		
	foreach(flavor ${flavors})
	    install( PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/csharp/${flavor}/Shell.BasinModeling.DataAccess.dll
		    DESTINATION	bin
	    )
	endforeach(flavor)
		
		
	###### Compile and run unit tests
	configure_file(test/AssemblyInfo.cs.cmake csharp-test/Properties/AssemblyInfo.cs)
	configure_file(test/SerialDataAccessTest.cs csharp-test/SerialDataAccessTest.cs)
	configure_file(Shell.BasinModeling.DataAccess.Test.csproj.cmake ${CMAKE_CURRENT_BINARY_DIR}/csharp-test/Shell.BasinModeling.DataAccess.Test.csproj @ONLY)
	
	if ( BM_WINDOWS_PLATFORM STREQUAL x64 )
	  set( TESTSETTINGS_EXECUTION_PARAMS hostProcessPlatform="MSIL" ) 
	  # Adding this attribute let's MSTest test 64-bit code
	  # Note, see http://www.cmake.org/Wiki/CMake/Language_Syntax for the weird rules on quoting in CMake
	elseif ( BM_WINDOWS_PLATFORM STREQUAL Win32 )
	  set( TESTSETTINGS_EXECUTION_PARAMS )
	  # Clearing that attribute, forces MSTest to run in 32-bit mode.
	else()
		message("Error: Cannot run MSTest on other platforms than x64 and Win32")
		set( TESTSETTINGS_EXECUTION_PARAMS )
	endif()
	configure_file(test/Local.testsettings.cmake csharp-test/Local.testsettings)

	include_external_msproject(Shell.BasinModeling.DataAccess.Test ${CMAKE_CURRENT_BINARY_DIR}/csharp-test/Shell.BasinModeling.DataAccess.Test.csproj
		TYPE "{FAE04EC0-301F-11D3-BF4B-00C04F79EFBC}" # This GUID is a Windows C# project (see also http://msdn.microsoft.com/en-us/library/hb23x61k(v=vs.80).aspx ) 
		Shell.BasinModeling.DataAccess
		PLATFORM "${BM_WINDOWS_PLATFORM}"
	)
	
	
	# Run all tests that contain the word 'Test' on all configurations
	
	foreach( flavor ${flavors})
	    
		add_test(NAME "Shell.BasinModeling.DataAccess.Test.SerialDataAccess.${flavor}"
		   COMMAND "${MSTEST}" 
			  "/testcontainer:csharp-test/${flavor}/Shell.BasinModeling.DataAccess.Test.dll"
			  "/testsettings:${CMAKE_CURRENT_BINARY_DIR}/csharp-test/local.testsettings"
			  "/test:Test"
			  "/publish:${BM_TFS_SERVER_URL}"
			  "/publishbuild:${BM_TFS_BUILD_NUMBER}"
			  "/flavor:${flavor}"
			  "/platform:${BM_WINDOWS_PLATFORM}"
			  "/teamproject:${BM_TFS_PROJECT_NAME}"
		)
		
		# Extend path so that it can find the SerialDataAccess.dll
		#get_target_property(SDA_LIB_PATH SerialDataAccess LOCATION)
	    #get_filename_component(SDA_LIB_DIR "${SDA_LIB_PATH}" PATH)
		
		#message("****************** SerialDataAcces.dll = ${SDA_LIB_DIR} ")
		set_tests_properties( "Shell.BasinModeling.DataAccess.Test.SerialDataAccess.${flavor}"
		  PROPERTIES ENVIRONMENT "PATH=${CMAKE_CURRENT_BINARY_DIR}/${flavor};$ENV{PATH}"
		)
	endforeach(flavor)
	   
	
else (BM_CSHARP_API)
	
	add_library(SerialDataAccess 
		${SDA_SOURCES}
	)
	
	target_link_libraries(SerialDataAccess ${SDA_LIBS} )
	
	if (UNIX)
		target_link_libraries(SerialDataAccess rt )
	endif(UNIX)			

	install(TARGETS SerialDataAccess
		RUNTIME DESTINATION bin
		LIBRARY	DESTINATION lib
		ARCHIVE DESTINATION lib
	)
	
endif (BM_CSHARP_API)	

