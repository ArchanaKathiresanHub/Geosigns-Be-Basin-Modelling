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

set(target_name grid2grid)

file(GLOB SOURCE_FILES "${BB_ROOT}/${target_name}/src/*.[Ch]")

add_definitions(-DMODULENAME="${target_name}" -DMODULEVER="1.0")
add_definitions(-Dgocad_defined_bool)

include_directories(${BB_ROOT}/RubyGeneralIO/src)
include_directories(${BB_ROOT}/RubyUtilities/src)
include_directories(${GEOCASE_BASE_DIR}/include/gospel)
include_directories(${GEOCASE_BASE_DIR}/include/dds)

link_directories(
  /nfs/rvl/users/ibs/geocase_64/prod/v2.4/lib/Linux64g/static
)

add_executable(${target_name} ${SOURCE_FILES})

target_link_libraries(${target_name} 
  RubyGeneralIO
  RubyUtilities
  Serial_Hdf5
  gospel dds dds
  Xt
  X11
)

install(TARGETS ${target_name}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
)

# end of file
