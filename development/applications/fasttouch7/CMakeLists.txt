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

if (BM_PARALLEL AND TSLIB_ROOT)

add_executable(fasttouch7
	src/main.C
	src/misc.C
	src/FastTouch.C
	src/MasterTouch.C
	src/BurialHistory.C
	src/WriteBurial.C
	src/TouchstoneFiles.C
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

include_directories(
	${HDF5_INCLUDE_DIRS}
	${MPI_INCLUDE_DIRS}
	${PETSC_INCLUDE_DIRS}
	${XERCES_INCLUDE_DIRS}
	${XSD_INCLUDE_DIRS}
	${TSLIB_INCLUDE_DIRS}
)

target_link_libraries(fasttouch7
	DataAccess
	DistributedDataAccess
	Utilities_Petsc
	Serial_Hdf5
	Parallel_Hdf5
	TableIO
	utilities
	CBMGenerics

	${HDF5_LIBRARIES}
	${PETSC_LIBRARIES}	
)

if (UNIX)
   target_link_libraries(fasttouch7
         OneFilePerProcess
   )
endif(UNIX)

set_target_properties(fasttouch7
      PROPERTIES LINK_FLAGS "${PETSC_LINK_FLAGS}"
)

if (BM_USE_FLEXLM)
   include_directories(${FLEXLM_INCLUDE_DIRS})
   target_link_libraries(fasttouch7 ${FLEXLM_LIBRARIES})
endif(BM_USE_FLEXLM)

install(TARGETS fasttouch7
	RUNTIME DESTINATION bin
)

add_definitions(-DMPICH_IGNORE_CXX_SEEK)
if (WIN32)
  #Min and Max defined in BurialHistorySchema.hxx collides with the min max macro
  add_definitions(-DNOMINMAX)
endif()

#Touchstone Wrapper

configure_file(touchstoneWrapper.sh.cmake "aux/touchstoneWrapper.sh")
file(COPY "${CMAKE_CURRENT_BINARY_DIR}/aux/touchstoneWrapper.sh"
        DESTINATION "${CMAKE_CURRENT_BINARY_DIR}"
        FILE_PERMISSIONS
           OWNER_READ OWNER_WRITE OWNER_EXECUTE
           GROUP_READ GROUP_WRITE GROUP_EXECUTE
           WORLD_READ WORLD_EXECUTE
)

add_executable(touchstoneWrapper
	src/TouchstoneWrapper.C 
	src/TsLibLoader.C
	src/LibLoaderLinux.C
	src/Touchstone.C
	src/ReadBurial.C
	src/TouchstoneFiles.C
)

target_link_libraries(touchstoneWrapper
	${TSLIB_LIBRARIES}
	${XERCES_LIBRARIES}
)

install(TARGETS touchstoneWrapper
	RUNTIME DESTINATION bin
)

install(PROGRAMS touchstoneWrapper.sh
   DESTINATION bin
)

### Unit Tests

add_gtest(NAME WriteReadBurialHistory
   SOURCES test/BurialHistory.C src/WriteBurial.C src/ReadBurial.C
   LIBRARIES ${TS7_LIBRARIES} ${TS_LIBRARIES} ${XERCES_LIBRARIES} 
)

add_gtest(NAME WriteReadTouchstone
   SOURCES test/Touchstone.C src/TouchstoneFiles.C
   LIBRARIES ${TS7_LIBRARIES} ${TS_LIBRARIES} ${XERCES_LIBRARIES} 
)

endif(BM_PARALLEL AND TSLIB_ROOT)

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
