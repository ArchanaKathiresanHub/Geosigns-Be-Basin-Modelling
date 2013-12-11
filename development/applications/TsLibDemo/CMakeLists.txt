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

if (BM_PARALLEL)
set(target_name TouchLibDemo)

add_executable(${target_name}
   src/codetimer.cpp
   src/libloaderlinux.cpp
   src/TsLibCppTester.cpp
   src/tslibloader.cpp
)

bm_include_libraries(
)

remove_definitions(-fPIC)
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

# IMPORTANT: remove -rdynamic from CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

include_directories(
	${XERCES_INCLUDE_DIRS}
	${XSD_INCLUDE_DIRS}
	${TS7_INCLUDE_DIRS}
	${TS_INCLUDE_DIRS}
)

target_link_libraries(${target_name} 

	${TS7_LIBRARIES}
	${TS_LIBRARIES}
	${XERCES_LIBRARIES}
)

install(TARGETS TouchLibDemo
	RUNTIME DESTINATION bin
)

endif(BM_PARALLEL)
