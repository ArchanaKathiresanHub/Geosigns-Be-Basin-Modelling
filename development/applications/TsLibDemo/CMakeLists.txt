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
set(target_name TouchLibDemo)

add_executable(${target_name}
   src/codetimer.cpp
   src/libloaderlinux.cpp
   src/TsLibCppTester.cpp
   src/tslibloader.cpp
)


# IMPORTANT: remove -rdynamic from CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

include_directories( SYSTEM
	${XERCES_INCLUDE_DIRS}
	${XSD_INCLUDE_DIRS}
	${TS7_INCLUDE_DIRS}
	${MPI_INCLUDE_DIRS}
)

target_link_libraries(${target_name} 
	${TS7_LIBRARIES}
	${XERCES_LIBRARIES}
	${MPI_LIBRARIES}
)

install(TARGETS TouchLibDemo
	RUNTIME DESTINATION bin
)

endif(BM_PARALLEL AND TS7_ROOT)
