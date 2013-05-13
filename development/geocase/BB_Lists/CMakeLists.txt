#
#
#

cmake_minimum_required(VERSION 2.8)

project(geocase)

option(GEOCASE_GOCAD_ENABLED "Enable Gocad" OFF)
option(GEOCASE_HDF5_ENABLED "Enable HDF5" ON)
option(GEOCASE_BUILD_IBSLINK "Build ibslink" OFF)
option(GEOCASE_BUILD_123KEY "Build 123key" OFF)
option(GEOCASE_BUILD_GRID2GRID "Build grid2grid" ON)
option(GEOCASE_BUILD_PROJECT2HDF "Build project2hdf" ON)
option(GEOCASE_BUILD_VIEW4D "Build view4D" OFF)
option(GEOCASE_BUILD_DOCUMENTATION "Build documentation" ON)

get_filename_component(BB_ROOT "../BB" ABSOLUTE)

macro(generate_dox dox)
	if(GEOCASE_BUILD_DOCUMENTATION)
		#    message (WARNING "TODO: macro generate_dox is not defined yet (${dox})")
	endif()
endmacro()

# Redefine MACRO from Basin Modeling Utilities & Functions
macro( bm_include_libraries )
    foreach(library ${ARGN})
        include_directories(
            ${BB_ROOT}/../../libraries/${library}/src
            ${PROJECT_BINARY_DIR}/libraries/${library}
            )
    endforeach(library)
endmacro( bm_include_libraries )

set(GEOCASE_BASE_DIR "/nfs/rvl/users/ibs/geocase_64/prod/v2.4")

if(GEOCASE_HDF5_ENABLED)
	set(HDF5_VERSION "1.6.6" CACHE STRING "HDF5 Version")
	set(HDF5_ROOT "/nfs/rvl/groups/ept-sg/SWEast/Cauldron/hpc/hdf5/${HDF5_VERSION}/LinuxRHEL64_x86_64_57WS")
	set(HDF5_INCLUDE_DIRS ${HDF5_ROOT}/include)
	set(HDF5_serial_INCLUDE_DIRS ${HDF5_INCLUDE_DIRS})

	list(APPEND HDF5_LIBRARIES ${HDF5_ROOT}/lib/libhdf5.a)
	list(APPEND HDF5_LIBRARIES z)
	set(HDF5_serial_LIBRARIES ${HDF5_LIBRARIES})

	message(STATUS "HDF5_LIBRARIES = ${HDF5_LIBRARIES}")
	message(STATUS "HDF5_INCLUDE_DIRS = ${HDF5_INCLUDE_DIRS}")
endif()

# libraries
add_subdirectory(RubyUtilities)
add_subdirectory(RubyGeneralIO)

if(GEOCASE_HDF5_ENABLED)
	# the same CMakeLists.txt is used for Basin modeling and for BB
	add_subdirectory(../../libraries/Serial_Hdf5 Serial_Hdf5)
endif()

add_subdirectory(../../libraries/TableIO TableIO)
add_subdirectory(../../libraries/CBMGenerics CBMGenerics)
add_subdirectory(../../libraries/EosPack EosPack)

# applications

if(GEOCASE_BUILD_IBSLINK)
	add_subdirectory(ibslink)
endif()

if(GEOCASE_BUILD_123KEY)
	add_subdirectory(123key)
endif()

if(GEOCASE_BUILD_GRID2GRID)
	add_subdirectory(grid2grid)
endif()

if(GEOCASE_BUILD_VIEW4D)
	set(OPENINVENTOR_BASE_DIR "/nfs/rvl/groups/ept-sg/SWEast/Cauldron/OpenInventor/OpenInventor_6.1.2/Linux64_24_gcc3.4.6" CACHE PATH "OpenInventor base directory")
	add_subdirectory(view4D)
endif()

if(GEOCASE_BUILD_PROJECT2HDF)
	add_subdirectory(project2hdf)
endif()

# installation

install(DIRECTORY ../misc/ # last backslach is MANDATORY
	DESTINATION misc
    FILE_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ
    DIRECTORY_PERMISSIONS 
        OWNER_READ OWNER_WRITE OWNER_EXECUTE 
        GROUP_READ GROUP_WRITE GROUP_EXECUTE 
        WORLD_READ WORLD_EXECUTE
	PATTERN "../misc/gempis/*" 
	    PERMISSIONS
		    OWNER_READ OWNER_WRITE OWNER_EXECUTE 
		    GROUP_READ GROUP_WRITE GROUP_EXECUTE 
            WORLD_READ WORLD_EXECUTE
    )

file(GLOB openinventor_font_files 
	${OPENINVENTOR_BASE_DIR}/data/fonts/Utopia*
	${OPENINVENTOR_BASE_DIR}/data/fonts/Helvetica*
	)

install(FILES ${openinventor_font_files} 
	DESTINATION misc
	)

file(GLOB openinventor_so_files  ${OPENINVENTOR_BASE_DIR}/lib/*.so)

install(FILES ${openinventor_so_files}
	DESTINATION geocase64
	)

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
