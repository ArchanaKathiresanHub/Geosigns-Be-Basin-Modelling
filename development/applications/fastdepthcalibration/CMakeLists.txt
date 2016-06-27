#########################################################################
#                                                                       #
# Copyright (C) 2012-2016 Shell International Exploration & Production. #
# All rights reserved.                                                  #
#                                                                       #
# Developed under license for Shell by PDS BV.                          #
#                                                                       #
# Confidential and proprietary source code of Shell.                    #
# Do not distribute without written permission from Shell.              #
#                                                                       #
#########################################################################

if (BM_PARALLEL)

################################################################
######################### SET TARGETS ##########################
################################################################

source_group(source FILES src/main.C)
add_executable(fastdepthcalibration src/main.C )

################################################################
################### SET DEPENDENCIES/LINKS #####################
################################################################

if (UNIX)
add_dependencies(fastdepthcalibration FastcauldronApplication TableIO Boost cmbapi PETSC )
else (WIN32)
add_dependencies(fastdepthcalibration FastcauldronApplication TableIO )
endif (UNIX)

include_directories(
   src
   ${PROJECT_SOURCE_DIR}/applications/fastcauldron/src
   ${PROJECT_SOURCE_DIR}/libraries/cmbAPI/src
   ${HDF5_INCLUDE_DIRS}
   ${MPI_INCLUDE_DIRS}
   ${PETSC_INCLUDE_DIRS}
   ${EIGEN_INCLUDE_DIRS}
)

bm_include_libraries(
	FastcauldronApplication
	DataModel
	DataAccess
	DerivedProperties
	DistributedDataAccess
	utilities
	Utilities_Petsc
	Serial_Hdf5
	Parallel_Hdf5
	EosPack
	TableIO
	LinearAlgebra
	Interpolation
	FiniteElements
	CBMGenerics
	genex6
	genex6_kernel	
	GeoPhysics
	FileSystem
	calibration_kernel
    cmbapi	
)

target_link_libraries(fastdepthcalibration
	FastcauldronApplication
	DataAccess
	DistributedDataAccess
	)

target_link_libraries( fastdepthcalibration
	DataAccess
	DerivedProperties
	DistributedDataAccess
	utilities
	Utilities_Petsc
	Serial_Hdf5
	Parallel_Hdf5
	EosPack
	TableIO
	LinearAlgebra
	Interpolation
	FiniteElements
	CBMGenerics
	genex6
	genex6_kernel	
	GeoPhysics
	FileSystem
	calibration_kernel
	OTGC_kernel6
	cmbapi
	${HDF5_LIBRARIES}
	${PETSC_LIBRARIES}
	${MPI_LIBRARIES}
	${Boost_LIBRARIES}
    ${BM_CLOCK_GETTIME_LIB}
)

set_target_properties(fastdepthcalibration
      PROPERTIES LINK_FLAGS "${PETSC_LINK_FLAGS}"
)

if(UNIX)
   # The OneFilePerProcess library is only available on Unix
   bm_include_libraries( OneFilePerProcess )
   target_link_libraries(fastdepthcalibration OneFilePerProcess)
endif()

if (WIN32)
  # Windows.h defines 'min' and 'max' macros if NOMINMAX is not defined. These
  # macros collide with std::min and std::max
  add_definitions(-DNOMINMAX)
endif()

if (BM_USE_FLEXLM)
   include_directories( ${FLEXLM_INCLUDE_DIRS})
   target_link_libraries(fastdepthcalibration ${FLEXLM_LIBRARIES})
endif(BM_USE_FLEXLM)

install(TARGETS fastdepthcalibration
	RUNTIME DESTINATION bin
)
	
endif(BM_PARALLEL)

