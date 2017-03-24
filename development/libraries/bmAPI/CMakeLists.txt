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

# Building the TableIO library
#
# ATTENTION: The file is used by Basin Modeling and Geocase.
#
include_directories(src)

set (generator_source_files
  ${CMAKE_CURRENT_BINARY_DIR}/cauldronschemaAPI.h
  ${CMAKE_CURRENT_BINARY_DIR}/cauldronschemaAPI.cpp
)

add_library(bmAPI
	${generator_source_files}
)

# under Linux we are building Boost that is why this target exist
if (UNIX)
add_dependencies(bmAPI Boost)
endif (UNIX)

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR}/../TableIO )

add_dependencies(bmAPI
   TableIO
   FileSystem
   utilities
)

bm_include_libraries( bmAPI
   TableIO
   FileSystem
   utilities
)

target_link_libraries( bmAPI
   TableIO
   FileSystem
   utilities
)

add_custom_command( OUTPUT ${generator_source_files}
	DEPENDS parspec ../TableIO/BasinModellerProperty.Spec
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	COMMAND parspec -schema CauldronSchema -directory ${CMAKE_CURRENT_BINARY_DIR} -input ../TableIO/BasinModellerProperty.Spec -bmapi
)

install(TARGETS bmAPI
	RUNTIME DESTINATION bin
	LIBRARY	DESTINATION lib
	ARCHIVE DESTINATION lib
)

configure_file(
      ${CMAKE_CURRENT_SOURCE_DIR}/test/Reference.project3d
      ${CMAKE_CURRENT_BINARY_DIR}
      COPYONLY
)

add_gtest( NAME TableIO::CauldronSchemaAPITest
           SOURCES test/CauldronSchemaAPITest.cpp
           LIBRARIES bmAPI TableIO utilities FileSystem
)
