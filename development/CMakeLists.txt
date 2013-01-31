#
#
#

cmake_minimum_required(VERSION 2.8)
Project(BasinModeling)

#
# Configurable options
#

option( BM_CSHARP_API "Build the C sharp interface (Windows only)" ${WIN32} )
option( BM_PARALLEL "Build the parallel cauldron applications" ${UNIX} )
option( BM_USE_OWN_MPICH_LOCK "Use our custom ADIOI_Set_lock functions in MPI library (Linux only)" ON )
option( BM_BUILD_COMPLEMENTARY_APPS "Also build complementary apps" ON )
option( BM_BUILD_SERIAL_APPS "Also build serial version of apps" ON )
option( BUILD_SHARED_LIBS "Build shared libraries" OFF )
option( BM_PARALLEL "Build the parallel basin modeling applications" ON )
option( BM_BUILD_DOCS "Build documentation" OFF )
option( BM_BUILD_ENABLE_TESTING "Whether testing is enabled" ON )
option( BM_CODE_COVERAGE_ENABLE "Enable code coverage" OFF )
if(BM_CODE_COVERAGE_ENABLE)
	set( BM_CODE_COVERAGE_DIR "${PROJECT_BINARY_DIR}/profiled" CACHE PATH "Code coverage directory" )
endif()

option( BM_BUILD_CBMGENERICS_STATIC "Build CBMGENERICS library STATIC" ON )
option( BM_BUILD_EOSPACK_STATIC "Build EOSPACK library STATIC" OFF )
option( BM_BUILD_TABLEIO_STATIC "Build TABLEIO library STATIC" ON )
option( BM_BUILD_UTILITIES_STATIC "Build utilities library STATIC" ON )

set(CMAKE_INSTALL_PREFIX "" CACHE PATH "Installation prefix" )
set(CMAKE_BUILD_TYPE "" CACHE STRING "CMake build type ('Release' or 'Debug')")
set(SSSDEV_INSTALL_DIR "/apps/sssdev/ibs" CACHE PATH "sssdev installation path")

set(BM_VERSION_NUMBER_MAJOR "2013" CACHE STRING "Numerical version major string. Anything like 2013")
set(BM_VERSION_NUMBER_MINOR "01" CACHE STRING "Numerical version minor string. Anything like 01")
set(BM_VERSION_NUMBER ${BM_VERSION_NUMBER_MAJOR}.${BM_VERSION_NUMBER_MINOR})
set(BM_VERSION_TAG "nightly" CACHE STRING "Version tag. E.g. nightly, cmake, etc")
set(BM_SVNREVISION "-9999" CACHE STRING "Source control revision number (TFS or SVN)")

set(BM_TFS_BUILD_NUMBER "" CACHE STRING "TFS Build identifier (aka BuildDetail.BuildNumber)")
set(BM_TFS_SERVER_URL "" CACHE STRING "TFS Collection URL of Cauldron")
set(BM_TFS_PROJECT_NAME "" CACHE STRING "TFS Project Name (i.e. 'Basin Modeling'")

#set(BM_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/Binaries" CACHE PATH "Output directory (MSBuild uses this variable to pass OutDir build parameter)")

if(NOT BM_BUILD_NUMBER)
    set(BM_BUILD_NUMBER 0)
endif()

if(NOT BM_REVISION)
	set(BM_REVISION 0)
endif()

set(BM_VERSION_STRING "v${BM_VERSION_NUMBER}${BM_VERSION_TAG}")

if(NOT CMAKE_BUILD_TYPE)
	set (CMAKE_BUILD_TYPE Release)
endif()

if(BM_BUILD_ENABLE_TESTING)
	enable_testing()
endif()

include(files/PredefinedVariables.cmake)

if(WIN32)
	if (CMAKE_CL_64)
		set(BM_WINDOWS_PLATFORM "x64")
	else (CMAKE_CL_64)
		set(BM_WINDOWS_PLATFORM "Win32")
	endif (CMAKE_CL_64)
	# remove  /Zm1000 from command line options
	string(REPLACE "/Zm1000" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
	string(REPLACE "/Zm1000" "" CMAKE_C_FLAGS ${CMAKE_CXX_FLAGS})
endif()

message(STATUS "BM_WINDOWS_PLATFORM = ${BM_WINDOWS_PLATFORM}")

# Modules
# include(GenerateExportHeader)
include(FeatureSummary)

include(files/BasinModelingUtilities.cmake)

### Visual Studio 
if (MSVC)
	add_definitions( "-Dand=&&" "-Dor=||" "-Dnot=!" )
	option( BM_USE_MSVC_DYNAMIC_LIBS "Use the dynamic C runtime" ON )
#	if (BM_USE_MSVC_DYNAMIC_LIBS)
#		configure_msvc_runtime("dynamic")
#	else()
#		configure_msvc_runtime("static")
#	endif()
    
	find_program( MSTEST "mstest.exe"
	  PATHS "C:/Program Files (x86)/Microsoft Visual Studio 10.0/Common7/IDE"
	        "C:/Program Files (x86)/Microsoft Visual Studio 11.0/Common7/IDE"
	  DOC "Location of MSTest unit test driver"
	)
endif ()

if(UNIX)
	# The library to be compiled with -O3 is EosPack, all others, 
	# including the applications are to be compiled with -O2.
	# Also we want assert(3) in Release configuration
	set(CMAKE_CXX_FLAGS_RELEASE "-O2")
	# IMPORTANT: somehow -fPIC affects regression test results
	add_definitions(-fPIC)
	if(BM_CODE_COVERAGE_ENABLE)
		file(MAKE_DIRECTORY ${BM_CODE_COVERAGE_DIR})
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -prof-gen=srcpos -prof-dir \"${BM_CODE_COVERAGE_DIR}\"")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -g")
	endif()
endif()

### SWIG
if (BM_CSHARP_API)
	find_package(SWIG REQUIRED)
	include(UseSWIG)
endif()

### MPI

if (BM_PARALLEL)
	find_package(MPI REQUIRED)
	# Note: We only need C bindings. The C++ bindings sometimes give trouble
	# because of SEEK_SET, etc... already being defined in stdio.h or iostream
	# add_definitions(-DMPICH_SKIP_MPICXX)
	if (BM_USE_OWN_MPICH_LOCK)
		add_subdirectory(libraries/mpich2_lock)
		set(BM_MPI_LOCK_LIBRARIES)
		list(APPEND BM_MPI_LOCK_LIBRARIES "-Wl,--whole-archive" mpich2_lock "-Wl,--no-whole-archive")
	endif(BM_USE_OWN_MPICH_LOCK)
endif()

### HDF5
if (BUILD_SHARED_LIBS)
	set(HDF5_USE_STATIC_LIBRARIES OFF)
else(BUILD_SHARED_LIBS)
	set(HDF5_USE_STATIC_LIBRARIES ON)
endif(BUILD_SHARED_LIBS)

set (HDF5_VERSION  "1.8.7" CACHE STRING "HDF5 version")
include(files/FindHDF5.cmake)

if(NOT BUILD_SHARED_LIBRARIES)
	patch_mpi_libraries_to_use_static(MPI_C_LIBRARIES ${MPI_C_LIBRARIES})
endif()

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
if(UNIX)
	#Note to following line: the FindLAPACK modules dooes not provide
	# variables to specify search location, therefore I use here the general
	# 'CMAKE_PREFIX_PATH' variable.
	list(APPEND CMAKE_PREFIX_PATH ${BLAS_ROOT} ${LAPACK_ROOT})
	set(BLA_STATIC ON)
	find_package(BLAS)
	#find_package(LAPACK)
endif(UNIX)

include(files/FindPETSC.cmake)

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
if(UNIX)
	find_package(Qt3 REQUIRED)
	include(files/Qt3Macros.cmake)
endif(UNIX)

### QWT
find_path( QWT_INCLUDE_DIRS qwt.h
	HINTS "${QWT_ROOT}/include"
)
find_library( QWT_LIBRARIES qwt
	HINTS "${QWT_ROOT}/lib"
)

### Doxygen (for documentation generation)
if(BM_BUILD_DOCS)
	find_package(Doxygen)

	if(DOXYGEN_FOUND)
		add_custom_target(doc
			COMMENT "Generating Doxygen documentation"
		)
	endif()
endif()

### More definitions ###

add_definitions(-DIBSFLEXLMVERSION="2008.04")
add_definitions(-DIBSLONGVERSION="v${BM_VERSION_NUMBER}")
add_definitions(-DIBSSHORTVERSION="v${BM_VERSION_NUMBER}")
add_definitions(-DIBSLONGVERSION="v${BM_VERSION_NUMBER}")
add_definitions(-DSVNREVISION="${BM_SVNREVISION}")

if(WIN32)
	if(BM_BUILD_CBMGENERICS_STATIC)
		add_definitions(-DCBMGENERICS_STATIC)
	endif()

	if(BM_BUILD_EOSPACK_STATIC)
		add_definitions(-DEOSPACK_STATIC)
	endif()

	if(BM_BUILD_TABLEIO_STATIC)
		add_definitions(-DTABLEIO_STATIC)
	endif()

	if(BM_BUILD_UTILITIES_STATIC)
		add_definitions(-DUTILITIES_STATIC)
	endif()
endif()

### 

### Add subdirs
add_subdirectory(libraries)
add_subdirectory(applications)
if(BM_BUILD_REGRESSION_TESTS)
	add_subdirectory(regression_tests)
endif()

### Generate setup scripts

if (WIN32)
   set(script_suffix ".bat")
else ()
   set(script_suffix ".csh")
endif()

get_filename_component(CMAKE_COMMAND_PATH ${CMAKE_COMMAND} PATH)

configure_file(envsetup${script_suffix}.cmake envsetup${script_suffix})
configure_file(InstallAll${script_suffix}.cmake InstallAll${script_suffix} @ONLY)
configure_file(MoveInstalls${script_suffix}.cmake MoveInstalls${script_suffix} @ONLY)

### Feature reporting
feature_summary(WHAT ALL)

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
