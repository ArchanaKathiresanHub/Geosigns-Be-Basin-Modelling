#
#
#

cmake_minimum_required(VERSION 2.8.9)
Project(BasinModeling)

#
# Configurable options
#
option( BM_CSHARP_API "Build the C sharp interface" OFF )
option( BM_PARALLEL "Build the parallel cauldron applications" ON )
option( BM_USE_OWN_MPICH_LOCK "Use our custom ADIOI_Set_lock functions in MPI library (Linux only)" ON )
option( BM_BUILD_COMPLEMENTARY_APPS "Also build complementary apps" ON )
option( BM_BUILD_SERIAL_APPS "Also build serial version of apps" ON )
option( BUILD_SHARED_LIBS "Build shared libraries" OFF )
option( BM_PARALLEL "Build the parallel basin modeling applications" ON )

set(CMAKE_INSTALL_PREFIX "" CACHE PATH "Installation prefix" )
set(CMAKE_BUILD_TYPE "" CACHE STRING "CMake build type ('Release' or 'Debug')")

if(NOT CMAKE_BUILD_TYPE)
       set (CMAKE_BUILD_TYPE Release)
endif()

include(PredefinedVariables.cmake)

# Modules
include(GenerateExportHeader)
include(FeatureSummary)

include(BasinModelingUtilities.cmake)

### Visual Studio 
if (MSVC)
	add_definitions( "-Dand=&&" "-Dor=||" "-Dnot=!" )	
	option( BM_USE_MSVC_DYNAMIC_LIBS "Use the dynamic C runtime" ON )
	if (BM_USE_MSVC_DYNAMIC_LIBS)
		configure_msvc_runtime("dynamic")
	else()
		configure_msvc_runtime("static")
	endif()
endif (MSVC)

### SWIG
set( BM_SWIG_REQUIRED )
if (BM_CSHARP_API)
	set( BM_SWIG_REQUIRED REQUIRED )
endif()
  
find_package(SWIG ${BM_SWIG_REQUIRED})
include(UseSWIG)

### MPI

if (BM_PARALLEL)
  find_package(MPI REQUIRED)
  # Note: We only need C bindings. The C++ bindings sometimes give trouble
  # because of SEEK_SET, etc... already being defined in stdio.h or iostream
  add_definitions(-DMPICH_SKIP_MPICXX)
  if (BM_USE_OWN_MPICH_LOCK)
    add_subdirectory(libraries/mpich2_lock)
    list(INSERT MPI_C_LIBRARIES 0 "mpich2_lock")
  endif(BM_USE_OWN_MPICH_LOCK)
endif()

### HDF5
if (BUILD_SHARED_LIBS)
	set(HDF5_USE_STATIC_LIBRARIES OFF)
else(BUILD_SHARED_LIBS)
	set(HDF5_USE_STATIC_LIBRARIES ON)
endif(BUILD_SHARED_LIBS)

find_package(HDF5
	REQUIRED
	COMPONENTS C
)

# Note on weird notation: HDF5_IS_PARALLEL has values "TRUE" and "FALSE" while
# BM_PARALLEL can be "ON" and "OFF". By using 'NOT' operator these
# values are converted to the same range.
if ( NOT ( (NOT HDF5_IS_PARALLEL) EQUAL (NOT BM_PARALLEL) ) )
	message(WARNING "The parallelism of the HDF5 library didn't match the build request")
endif()

add_definitions(${HDF5_DEFINITIONS})

# include MPI libraries automaticaly when HDF5 library is parallel version
if (HDF5_IS_PARALLEL)
	list(APPEND HDF5_INCLUDE_DIRS "${MPI_C_INCLUDE_PATH}")
	list(APPEND HDF5_LIBRARIES "${MPI_C_LIBRARIES}")
endif(HDF5_IS_PARALLEL)

# BLAS, LAPACK, and PetSC

#Note to following line: the FindLAPACK modules dooes not provide
# variables to specify search location, therefore I use here the general
# 'CMAKE_PREFIX_PATH' variable.
list(APPEND CMAKE_PREFIX_PATH ${BLAS_ROOT} ${LAPACK_ROOT})
set(BLA_STATIC ON)
find_package(BLAS)
#find_package(LAPACK)

find_path(PETSC_INCLUDE_DIRS petscvec.h
	PATHS ${PETSC_ROOT}/include
	DOC "Include directory of PetSc"
)

find_library(PETSC_LIBRARIES petsc
	PATHS ${PETSC_ROOT}/lib
	DOC "Library directory of PetSc"
)
set(PETSC_FOUND)
if (PETSC_INCLUDE_DIRS AND PETSC_LIBRARIES)
	set(PETSC_FOUND YES)
else()
	set(PETSC_FOUND NO)
	message("Petsc library could not be found.")
endif()

### FLexLM
set( FLEXLM_LIBRARIES "FLEXLM_LIBRARIES-NOTFOUND" CACHE PATH "Path of FlexLM libraries" )
set( FLEXLM_INCLUDE_DIRS "FLEXLM_INCLUDE_DIRS-NOTFOUND" CACHE PATH "Path of FlexLM include files" )

### Function Parser
set( FP_ROOT "FP_ROOT-NOTFOUND" CACHE PATH "Path to Function Parser library (see also http://warp.povusers.org/FunctionParser/ )" )
set( FP_LIBRARIES "FP_LIBRARIES-NOTFOUND" CACHE PATH "Path to Function Parser library" )
set( FP_INCLUDE_DIRS "FP_INCLUDE_DIRS-NOTFOUND" CACHE PATH "to Function Parser library include directory" )


### Touchstone
set( TS_INCLUDE_DIRS "TS_INCLUDE_DIRS-NOTFOUND" CACHE PATH "" )
set( TS_LIBRARIES "TS_LIBRARIES-NOTFOUND" CACHE PATH "" )

### Touchstone 7.0
set( TS7_ROOT "TS7_ROOT-NOTFOUND" CACHE PATH "")
set( TS7_GEOCOSM_XML_INCLUDE "${TS7_ROOT}/geocosmxml" CACHE PATH "")
set( TS7_INCLUDE_DIRS "${TS7_ROOT}/include" CACHE PATH "" )
set( TS7_LIBRARY_DIR "${TS7_ROOT}/lib" CACHE PATH "" )
set( TS7_LIBRARIES "-L${TS7_LIBRARY_DIR}  -lgeocosmxmllibbasecpp -lgeocosmexception" CACHE PATH "" )

### XSD (see http://www.codesynthesis.com/projects/xsd/ )
set( XSD_ROOT "XSD-NOTFOUND" CACHE PATH "")
set( XSD_INCLUDE_DIRS "${XSD_ROOT}/libxsd" CACHE PATH "")

### Xerces-C
find_path( XERCES_INCLUDE_DIRS "xercesc/dom/DOM.hpp" 
	HINTS "${XERCES_ROOT}/include"
	DOC "Xerces-C include directory"
)

find_library( XERCES_LIBRARIES "xerces-c"
	HINTS "${XERCES_ROOT}/lib"
	DOC "Xerces-C libraries"
)

### QT3
find_package(Qt3 REQUIRED)

# A special macro is defined here, to have better control
# on the names of intermediate files.
macro( qt3_wrap_cpp OUTPUT_FILES MOC_EXTENSION)
	set(${OUTPUT_FILES})
	foreach( file  ${ARGN})
		set(input ${file})
		# Remove any directories and replace the .hpp or .h extension with .moc.cpp 
		string(REGEX REPLACE "(.*\\/)?([^.]*)\\.h(pp)?$" "\\2${MOC_EXTENSION}" output "${input}")
		add_custom_command( 
			OUTPUT ${output}
			COMMAND ${QT_MOC_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/${input} -o ${output}
			DEPENDS ${input}
		)
		list(APPEND ${OUTPUT_FILES} ${output})	
	endforeach(file)
endmacro(qt3_wrap_cpp)

macro( qt3_wrap_ui OUTPUT_FILES HEADER_EXTENSION IMPL_EXTENSION MOC_EXTENSION)
	set(${OUTPUT_FILES})
	foreach( file  ${ARGN})
		set(input ${file})
		# Remove any directories and replace the .ui extension with .hpp
		string(REGEX REPLACE "(.*\\/)?([^.]*)\\.ui$" "\\2${HEADER_EXTENSION}" header "${input}")
		# Remove any directories and replace the .ui extension with .cpp
		string(REGEX REPLACE "(.*\\/)?([^.]*)\\.ui$" "\\2${IMPL_EXTENSION}" impl   "${input}")
		# Remove any directories and replace the .ui extension with .moc.cpp
		string(REGEX REPLACE "(.*\\/)?([^.]*)\\.ui$" "\\2${MOC_EXTENSION}" moc_impl "${input}")
		add_custom_command( 
			OUTPUT ${header} ${impl} ${moc_impl}
			COMMAND ${QT_UIC_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/${input} -o ${header}
			COMMAND ${QT_UIC_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/${input} -impl ${header} -o ${impl}
			COMMAND ${QT_MOC_EXECUTABLE} ${header} -o ${moc_impl}
			DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${input}
		)
		list(APPEND ${OUTPUT_FILES} ${impl} ${moc_impl})	
	endforeach(file)
endmacro(qt3_wrap_ui)

macro( qt3_wrap_images OUTPUT_FILES PROJECT_NAME )
	set(${OUTPUT_FILES})
	set(qt3_image_wrap_file "cmake_image_collection.cpp")
	set(${OUTPUT_FILES} ${qt3_image_wrap_file})
	add_custom_command(
		OUTPUT ${qt3_image_wrap_file}
		COMMAND ${QT_UIC_EXECUTABLE} -embed ${PROJECT_NAME} ${ARGN} -o ${CMAKE_CURRENT_BINARY_DIR}/${qt3_image_wrap_file}
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		DEPENDS ${ARGN}
	)
endmacro(qt3_wrap_images)


### QWT
find_path( QWT_INCLUDE_DIRS qwt.h
	HINTS "${QWT_ROOT}/include"
)
find_library( QWT_LIBRARIES qwt
	HINTS "${QWT_ROOT}/lib"
)	

### Doxygen (for documentation generation)
find_package(Doxygen)

add_custom_target(doc
	COMMENT "Generating Doxygen documentation"
)

### Add subdirs
add_subdirectory(libraries)
add_subdirectory(applications)

### Generate setup scripts

if (WIN32)
   set(envsetup_script_name "envsetup.bat")
else ()
   set(envsetup_script_name "envsetup.csh")
endif()

get_filename_component(CMAKE_COMMAND_PATH ${CMAKE_COMMAND} PATH)

configure_file(${envsetup_script_name}.cmake ${envsetup_script_name})  

### Feature reporting
feature_summary(WHAT ALL)
