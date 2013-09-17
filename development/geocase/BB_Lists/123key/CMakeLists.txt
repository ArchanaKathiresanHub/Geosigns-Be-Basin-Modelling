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

#
#
#

set(target_name 123key)


file(GLOB SOURCE_FILES "${BB_ROOT}/${target_name}/src/*.C")
file(GLOB HEADER_FILES "${BB_ROOT}/${target_name}/src/*.h")

include_directories(${PROJECT_BINARY_DIR}/TableIO
  ${BB_ROOT}/../../libraries/TableIO/src
)

add_executable(${target_name} ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(${target_name} TableIO)

install(TARGETS ${target_name}
        RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)

# end of file
