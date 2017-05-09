#########################################################################
#                                                                       #
# Copyright (C) 2012-2013 Shell International Exploration & Production. #
# All rights reserved.                                                  #
#                                                                       #
# Developed under license for Shell by PDS BV.                          #
#                                                                       #
# Confidential and proprietary source code of Shell.                    #
# Do not distribute without written permission from Shell.              #
#                                                                       #
#########################################################################

if (GIT_FOUND)
   # Exctract Spec file version information
   # the commit's SHA1, and whether the building workspace was dirty or not
   execute_process(COMMAND 
      "${GIT_EXECUTABLE}" log --format="%H" -1 VoxetProperty.Spec
      WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src"
      OUTPUT_VARIABLE SPEC_FILE_GIT_SHA1
      ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

   # the date of the commit
   execute_process(COMMAND
      "${GIT_EXECUTABLE}" log -1 --format=%ad --date=local VoxetProperty.Spec
      WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src"
      OUTPUT_VARIABLE SPEC_FILE_GIT_DATE
      ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

   # the subject of the commit
   execute_process(COMMAND
      "${GIT_EXECUTABLE}" log -1 --format=%s VoxetProperty.Spec
      WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src"
      OUTPUT_VARIABLE SPEC_FILE_GIT_COMMIT_SUBJECT
      ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
else(GIT_FOUND)
   set(SPEC_FILE_GIT_SHA1 "0000000000000000000000000000000000000000")  
   set(SPEC_FILE_GIT_DATE "Mon Jan 01 00:00:00 1900")
   set(SPEC_FILE_GIT_COMMIT_SUBJECT "Not found")
endif(GIT_FOUND)


# generate include file with Spec file version
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/SpecFileVersion.h.in" "${CMAKE_CURRENT_BINARY_DIR}/SpecFileVersion.h" @ONLY)

set (voxetProjectFormatSources
	${CMAKE_CURRENT_BINARY_DIR}/SpecFileVersion.h
	${CMAKE_CURRENT_BINARY_DIR}/voxetschemafuncs.h
	${CMAKE_CURRENT_BINARY_DIR}/voxetschemafuncs.cpp
	${CMAKE_CURRENT_BINARY_DIR}/voxetschema.h
	${CMAKE_CURRENT_BINARY_DIR}/voxetschema.cpp
	)

set( mainSources
	src/cauldron2voxet.cpp
	src/CauldronProperty.cpp
	src/DepthInterpolator.cpp
	src/GridDescription.cpp
	src/LayerInterpolator.cpp
	src/VoxetCalculator.cpp
	src/VoxetDomainInterpolator.cpp
	src/VoxetProjectHandle.cpp
	src/VoxetPropertyGrid.cpp
    )

add_executable( cauldron2voxet
   ${mainSources}
   ${voxetProjectFormatSources}
)

bm_include_libraries(
	DerivedProperties
	GeoPhysics
	DataAccess
	SerialDataAccess
	DataModel
	Interpolation
	LinearAlgebra
	EosPack
	utilities
	TableIO
	CBMGenerics
)

include_directories( ${PROJECT_BINARY_DIR}/applications/cauldron2voxet )

include_directories( SYSTEM	${HDF5_INCLUDE_DIRS} )

target_link_libraries(cauldron2voxet
	DerivedProperties
	DataMining
	GeoPhysics
	DataAccess
	SerialDataAccess
	Interpolation
	LinearAlgebra
	TableIO
	EosPack
	CBMGenerics
	utilities
	Serial_Hdf5
	${HDF5_LIBRARIES}
)

add_custom_command(OUTPUT ${voxetProjectFormatSources}
	DEPENDS parspec
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/applications/cauldron2voxet
	COMMAND parspec -schema VoxetSchema -directory ${PROJECT_BINARY_DIR}/applications/cauldron2voxet -input src/VoxetProperty.Spec
)

install(TARGETS cauldron2voxet
	RUNTIME DESTINATION bin
)

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
