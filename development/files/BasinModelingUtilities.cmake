#
#
#

if(BasinModelingUtilities_INCLUDED)
  return()
endif()
set(BasinModelingUtilities_INCLUDED TRUE)

# Basin Modeling Utilities & Functions
macro( bm_include_libraries )
	foreach(library ${ARGN})
		include_directories( 
			${PROJECT_SOURCE_DIR}/libraries/${library}/src
#			${PROJECT_BINARY_DIR}/libraries/${library}
		)
	endforeach(library)
endmacro( bm_include_libraries )

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
				WORKING_DIRECTORY ${CMAKE_CURRENT_SOUCE_DIR}/src
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

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
