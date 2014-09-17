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

if (BM_PARALLEL AND TS7_ROOT)

add_executable(fasttouch7
	src/main.C
        src/misc.C
	src/FastTouch.C
	src/MasterTouch.C
	src/ObjectFactory.C
	src/TsLibLoader.C
	src/LibLoaderLinux.C
)

bm_include_libraries(
	DataModel
	DistributedDataAccess
	DataAccess
	Utilities_Petsc
	Serial_Hdf5
	OneFilePerProcess
	Parallel_Hdf5
	TableIO
	utilities
	CBMGenerics
)

set(CMAKE_CXX_FLAGS_RELEASE "-O2")

# IMPORTANT: remove -rdynamic from CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

include_directories(
	${HDF5_INCLUDE_DIRS}
	${MPI_INCLUDE_DIRS}
	${PETSC_INCLUDE_DIRS}
	${XERCES_INCLUDE_DIRS}
	${XSD_INCLUDE_DIRS}
	${TS7_INCLUDE_DIRS}
)

target_link_libraries(fasttouch7
	DataAccess
	DistributedDataAccess
	Utilities_Petsc
	Serial_Hdf5
	OneFilePerProcess
	Parallel_Hdf5
	TableIO
	utilities
	CBMGenerics
	${TS7_LIBRARIES}
	${HDF5_LIBRARIES}
	${XERCES_LIBRARIES}
	${PETSC_LIBRARIES}
)

set_target_properties(fasttouch7
      PROPERTIES LINK_FLAGS "${PETSC_LINK_FLAGS}"
)

if (BM_USE_FLEXLM)
   include_directories( ${FLEXLM_INCLUDE_DIRS})
   target_link_libraries(fasttouch7 ${FLEXLM_LIBRARIES})
endif(BM_USE_FLEXLM)

install(TARGETS fasttouch7
	RUNTIME DESTINATION bin
)

endif(BM_PARALLEL AND TS7_ROOT)

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
