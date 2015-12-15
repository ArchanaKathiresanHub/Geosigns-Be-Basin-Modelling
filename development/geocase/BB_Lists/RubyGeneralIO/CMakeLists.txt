#########################################################################
#                                                                       #
# Copyright (C) 2012-2014 Shell International Exploration & Production. #
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
set(target_name RubyGeneralIO)

file(GLOB SOURCE_FILES "${BB_ROOT}/${target_name}/src/*.[Ch]")

include_directories(${BB_ROOT}/RubyUtilities/src)
include_directories("${HDF5_serial_INCLUDE_DIRS}")
include_directories("${GEOCASE_BASE_DIR}/include/dds")
include_directories("${GEOCASE_BASE_DIR}/include/gospel")

add_definitions(-DHDF5_ENABLED -DH5_USE_16_API)

add_library(${target_name} STATIC ${SOURCE_FILES})
