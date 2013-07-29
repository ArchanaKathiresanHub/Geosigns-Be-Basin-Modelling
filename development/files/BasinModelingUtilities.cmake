#
#
#

if(BasinModelingUtilities_INCLUDED)
  return()
endif()
set(BasinModelingUtilities_INCLUDED TRUE)

# Basin Modeling Utilities & Functions

#
# WITH_BINARY_DIR instruct to include binary directory of the library as well
#
macro( bm_include_libraries )
	foreach(library ${ARGN})
		if(${library} STREQUAL "TableIO")
			include_directories( ${PROJECT_BINARY_DIR}/libraries/${library} )
		endif()
		include_directories( ${PROJECT_SOURCE_DIR}/libraries/${library}/src )
	endforeach(library)
endmacro( bm_include_libraries )

macro( bm_include_bin_libraries )
	foreach(library ${ARGN})
		include_directories( 
			${PROJECT_BINARY_DIR}/libraries/${library}
		)
	endforeach(library)
endmacro( bm_include_bin_libraries )

# The following function is copied from 
# http://stackoverflow.com/questions/10113017/setting-the-msvc-runtime-in-cmake
macro(configure_msvc_runtime FLAVOR)
  if(MSVC)
       
    # Set compiler options.
    set(variables
      CMAKE_C_FLAGS_DEBUG
      CMAKE_C_FLAGS_MINSIZEREL
      CMAKE_C_FLAGS_RELEASE
      CMAKE_C_FLAGS_RELWITHDEBINFO
      CMAKE_CXX_FLAGS_DEBUG
      CMAKE_CXX_FLAGS_MINSIZEREL
      CMAKE_CXX_FLAGS_RELEASE
      CMAKE_CXX_FLAGS_RELWITHDEBINFO
    )
    if(${FLAVOR} STREQUAL "static")
      message(STATUS
        "MSVC -> forcing use of statically-linked runtime."
      )
      foreach(variable ${variables})
        if(${variable} MATCHES "/MD")
          string(REGEX REPLACE "/MD" "/MT" ${variable} "${${variable}}")
        endif()
      endforeach()
    else()
      message(STATUS
        "MSVC -> forcing use of dynamically-linked runtime."
      )
      foreach(variable ${variables})
        if(${variable} MATCHES "/MT")
          string(REGEX REPLACE "/MT" "/MD" ${variable} "${${variable}}")
        endif()
      endforeach()
    endif()
  endif()
endmacro()

macro( generate_dox DOXYGEN_CONFIG_FILE )
	if (BM_BUILD_DOCS AND DOXYGEN_FOUND)
		message(STATUS "We are here")
		# generate an identifier
		string(RANDOM LENGTH 10 id)
		file( MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc)
		if (UNIX)
			add_custom_target( doc_${id}
				# Override the OUTPUT_DIRECTORY config variable with a trick described in the doxygen FAQ
				COMMAND /bin/bash -c \"( cat ${CMAKE_CURRENT_SOURCE_DIR}/${DOXYGEN_CONFIG_FILE} \; echo 'OUTPUT_DIRECTORY=${CMAKE_CURRENT_BINARY_DIR}/doc'; ) | ${DOXYGEN_EXECUTABLE} -\"
				WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src
				COMMENT "Generating Doxygen documentation with ${DOXYGEN_CONFIG_FILE}"
			)
		endif(UNIX)
		if (WIN32)
			add_custom_target( doc_${id}
				# Override the OUTPUT_DIRECTORY config variable with a trick described in the doxygen FAQ
				COMMAND cmd.exe /c \"( type ${CMAKE_CURRENT_SOURCE_DIR}/${DOXYGEN_CONFIG_FILE} & echo OUTPUT_DIRECTORY=${CMAKE_CURRENT_BINARY_DIR}/doc; ) | ${DOXYGEN_EXECUTABLE} -\"
				WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src
				COMMENT "Generating Doxygen documentation with ${DOXYGEN_CONFIG_FILE}"
			)
		endif(WIN32)
		add_dependencies(doc "doc_${id}")
	endif()
endmacro(generate_dox)

macro (patch_mpi_libraries_to_use_static list_out)
  if(MPI_FOUND)
    set (_list)
    foreach(item ${ARGN})
      get_filename_component(_ext ${item} EXT)
      get_filename_component(_we ${item} NAME_WE)
      get_filename_component(_path ${item} PATH)

      if( "${_ext}" STREQUAL ".so")
		if (((${_we} STREQUAL "libmpigi") OR (${_we} STREQUAL "libmpigf") OR (${_we} STREQUAL "libmpi")))
          set(_a_libname "${_path}/${_we}.a")
	        if(EXISTS ${_a_libname})
			set(item ${_a_libname})
		  endif()
	  endif()
      endif()
      list(APPEND _list ${item})
    endforeach()
    set (${list_out} ${_list})
  endif()
endmacro (patch_mpi_libraries_to_use_static)

### Generate GUID
macro(new_guid Output)
  # Generate Project GUID
  set( Alphabet "0123456789ABCDEF")
  string( RANDOM LENGTH 8 ALPHABET ${Alphabet} one)
  string( RANDOM LENGTH 4 ALPHABET ${Alphabet} two)
  string( RANDOM LENGTH 4 ALPHABET ${Alphabet} three)
  string( RANDOM LENGTH 4 ALPHABET ${Alphabet} four)
  string( RANDOM LENGTH 12 ALPHABET ${Alphabet} five)
  
  set(${Output} "${one}-${two}-${three}-${four}-${five}")
endmacro(new_guid)
  


### Add C# Unit tests

macro(add_csharp_unittest )
    
	set(TestProjectName)
	set(Platform)
	set(Directory)
	set(Dependencies)
	set(TestList)
	set(TfsServerUrl)
	set(TfsBuildNumber)
	set(TfsProjectName)
	set(DeploymentItems)
	set(ProjectReferenceName)
	set(ProjectReferenceGuid)
	set(ProjectReferencePath)
	set(TestSourceFilesList)
	
	# parse parameters
	set(parameterName)
	set(parameterType)
	foreach(param ${ARGN})
	    if (param STREQUAL NAME)
	       set(parameterName TestProjectName)
		   set(parameterType Atom)
	    elseif(param STREQUAL PLATFORM)
		   set(parameterName Platform)
		   set(parameterType Atom)
		elseif(param STREQUAL DIRECTORY)
		   set(parameterName Directory)
		   set(parameterType Atom)
		elseif(param STREQUAL DEPENDS)
		   set(parameterName Dependencies)
		   set(parameterType Sequence)
		elseif(param STREQUAL TESTLIST)
		   set(parameterName TestList)
		   set(parameterType Sequence)
		elseif(param STREQUAL TFS_SERVER_URL)
		   set(parameterName TfsServerUrl)
		   set(parameterType Atom)
		elseif(param STREQUAL TFS_BUILD_NUMBER)
		   set(parameterName TfsBuildNumber)
		   set(parameterType Atom)
		elseif(param STREQUAL TFS_PROJECT_NAME)
		   set(parameterName TfsProjectName)
		   set(parameterType Atom)
		elseif(param STREQUAL DEPLOYMENT_ITEMS)
		   set(parameterName DeploymentItems)
		   set(parameterType Sequence)
		elseif(param STREQUAL PROJECT_REFERENCE)
		   set(parameterType ProjectReference)
		   set(parameterName ProjectReferenceName)
		elseif(param STREQUAL TEST_SOURCES)
		   set(parameterName TestSourceFilesList)
		   set(parameterType Sequence)
		elseif(parameterType STREQUAL Atom)
		   set(${parameterName} ${param})
		elseif(parameterType STREQUAL Sequence)
		   list(APPEND ${parameterName} ${param})
		elseif(parameterType STREQUAL ProjectReference)
		   set(${parameterName} ${param})
		   set(prn "ProjectReferenceName") # This and the two lines below are a workaround for CMake's silly if-statemetn
		   set(prg "ProjectReferenceGuid")
		   set(prp "ProjectReferencePath")
		   if (parameterName STREQUAL prn)
		     set(parameterName ProjectReferenceGuid)
		   elseif (parameterName STREQUAL prg)
		     set(parameterName ProjectReferencePath)
		   elseif (parameterName STREQUAL prp)
		     set(parameterName)
		   endif()
		endif()
	endforeach(param)

	if (MSVC)
        new_guid(Guid)
        set(ProjectReferences)
        if (ProjectReferenceName AND ProjectReferencePath AND ProjectReferenceGuid)
		  file(TO_NATIVE_PATH "${ProjectReferencePath}" ProjectReferencePath)
		  set(ProjectReferences "<ProjectReference Include='${ProjectReferencePath}'><Project>{${ProjectReferenceGuid}}</Project><Name>${ProjectReferenceName}</Name></ProjectReference>")
		endif()	
		
		if (TestSourceFilesList)
			set(PrefixedTestSourceFilesList)
		    foreach(fitem ${TestSourceFilesList} )
				set(NativeFullPathToFile)
				file( TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${fitem}" NativeFullPathToFile)
				list(APPEND PrefixedTestSourceFilesList "${NativeFullPathToFile}")
			endforeach(fitem)
			set(FileListForCompilation "${PrefixedTestSourceFilesList}")
		else(TestSourceFilesList)
		    set(FileListForCompilation "*.cs")
		endif(TestSourceFilesList)
		
        configure_file("${PROJECT_SOURCE_DIR}/files/CSharpTestProjectTemplate.csproj.cmake" "${Directory}/${TestProjectName}.csproj" @ONLY)
				
		## Write the local.testsettings file
		set(TestSettings "${CMAKE_CURRENT_BINARY_DIR}/${Directory}/local.testsettings")
		file(WRITE "${TestSettings}"
"<?xml version='1.0' encoding='UTF-8'?><TestSettings name='Local' id='4fd90fa6-1a9f-49c6-a25d-79db40300acb' xmlns='http://microsoft.com/schemas/VisualStudio/TeamTest/2010'> <Description>These are default test settings for a local test run.</Description><Deployment>")
  
        foreach (item ${DeploymentItems})
		   file(APPEND "${TestSettings}" "<DeploymentItem filename='${item}' />")
	    endforeach(item)
     
	    if ( Platform STREQUAL x64 )
		  set( TestSettingsExecutionParams hostProcessPlatform="MSIL" ) 
		  # Adding this attribute let's MSTest test 64-bit code
		  # Note, see http://www.cmake.org/Wiki/CMake/Language_Syntax for the weird rules on quoting in CMake
		elseif ( Platform STREQUAL Win32 )
		  set( TestSettingsExecutionParams )
		  # Clearing that attribute, forces MSTest to run in 32-bit mode.
		else()
			message("Error: Cannot run MSTest on other platforms than x64 and Win32")
			set( TestSettingsExecutionParams )
		endif()
	    file(APPEND "${TestSettings}" "</Deployment><Execution ${TestSettingsExecutionParams} ><TestTypeSpecific /><AgentRule name='LocalMachineDefaultRole'/></Execution></TestSettings>")
	
	    ## Include the generated C# project
		include_external_msproject("${TestProjectName}" "${CMAKE_CURRENT_BINARY_DIR}/${Directory}/${TestProjectName}.csproj"
			TYPE "{FAE04EC0-301F-11D3-BF4B-00C04F79EFBC}" # This GUID is a Windows C# project (see also http://msdn.microsoft.com/en-us/library/hb23x61k(v=vs.80).aspx ) 
			PLATFORM "${Platform}"
			GUID "${Guid}"
			${Dependencies}
		)


		# Add the test, and publish results if the necessary information is available
		
		if (TfsServerUrl AND TfsBuildNumber AND TfsProjectName)
		    add_test(NAME "${TestProjectName}"
			   COMMAND "${MSTEST}" 
				  "/testcontainer:${Directory}/$<CONFIGURATION>/${TestProjectName}.dll"
				  "/testsettings:${TestSettings}"
				  "/test:${TestList}"
				  "/publish:${TfsServerUrl}"
				  "/publishbuild:${TfsBuildNumber}"
				  "/flavor:$<CONFIGURATION>"
				  "/platform:${Platform}"
				  "/teamproject:${TfsProjectName}"
			)
		else()
		    add_test(NAME "${TestProjectName}"
			   COMMAND "${MSTEST}" 
				  "/testcontainer:${Directory}/$<CONFIGURATION>/${TestProjectName}.dll"
				  "/testsettings:${TestSettings}"
				  "/test:${TestList}"
			)
		endif()


	endif(MSVC)
endmacro(add_csharp_unittest)

### 

macro(add_gtest )

   ### Parse parameters
   set(testName)    # The name of the test
   set(sources)     # The source files
   set(libraries)   # The libraries that should be linked with it

   set(parameterName)
   foreach(param ${ARGN})
      if (param STREQUAL NAME)
         set(parameterName testName)
      elseif(param STREQUAL SOURCES)
         set(parameterName sources)
      elseif(param STREQUAL LIBRARIES)
         set(parameterName libraries)
      else()
         list(APPEND ${parameterName} ${param})
      endif()
   endforeach()

   ### Add the test
   # Make a suitable executable / target name by replacing funny charateres
   # with an underscore
   string( REGEX REPLACE "[^A-Za-z0-9]" "_" execName "Test${testName}")

   # Add Google Mock (which includes Google Test also) to the list of
   # libraries
   list(APPEND libraries "gmock_main")

   # Add the test executable with its sources
   add_executable( ${execName} ${sources})

   # Link with the necessary libraries
   target_link_libraries( ${execName} ${libraries})
   
   # Add the Google Mock and Google Test include directories
   get_property(incdirs TARGET ${execName} PROPERTY INCLUDE_DIRECTORIES)
   set_target_properties( ${execName} 
      PROPERTIES 
                 INCLUDE_DIRECTORIES "${GTEST_INCLUDE_DIRS};${GMOCK_INCLUDE_DIRS};${incdirs}"
   )

   # Add the test to the CTest test  collection.
   add_test(${testName} ${execName})
endmacro(add_gtest)

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
