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
	# generate version.rc resource file
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
	if (BM_OUTPUT_DIRECTORY)
		# bm_output_directory_native MUST contain trailing back-slash
		file(TO_NATIVE_PATH ${BM_OUTPUT_DIRECTORY}/${BM_WINDOWS_PLATFORM}/ bm_output_directory_native)
	else()
		# bm_output_directory_native MUST contain trailing back-slash
		file(TO_NATIVE_PATH ${CMAKE_CURRENT_BINARY_DIR}/${BM_WINDOWS_PLATFORM}/ bm_output_directory_native)
	endif()
	message(STATUS "bm_output_directory_native = ${bm_output_directory_native}")
	configure_file( Shell.BasinModeling.DataAccess.csproj.in ${CMAKE_CURRENT_BINARY_DIR}/csharp/Shell.BasinModeling.DataAccess.csproj @ONLY)
	
	include_external_msproject(Shell.BasinModeling.DataAccess ${CMAKE_CURRENT_BINARY_DIR}/csharp/Shell.BasinModeling.DataAccess.csproj
		TYPE "{FAE04EC0-301F-11D3-BF4B-00C04F79EFBC}" # This GUID is a Windows C# project (see also http://msdn.microsoft.com/en-us/library/hb23x61k(v=vs.80).aspx ) 
		PLATFORM "${BM_WINDOWS_PLATFORM}"
		SerialDataAccess
	)
	
	file(TO_CMAKE_PATH ${bm_output_directory_native} bm_output_directory_cmake)
	install( PROGRAMS ${bm_output_directory_cmake}/Debug/Shell.BasinModeling.DataAccess.dll
		DESTINATION	lib
		CONFIGURATIONS Debug
	)
	install( PROGRAMS ${bm_output_directory_cmake}/Release/Shell.BasinModeling.DataAccess.dll
		DESTINATION	lib
		CONFIGURATIONS Release
	)
	
	install(TARGETS SerialDataAccess
		RUNTIME DESTINATION bin
		LIBRARY	DESTINATION lib
	)

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
