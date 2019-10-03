#########################################################################
#                                                                       #
# Copyright (C) 2012-2018 Shell International Exploration & Production. #
# All rights reserved.                                                  #
#                                                                       #
# Confidential and proprietary source code of Shell.                    #
# Do not distribute without written permission from Shell.              #
#                                                                       #
#########################################################################

set( M1D_APP_NAME "modelpseudo1d" )
add_executable( ${M1D_APP_NAME} src/main.cpp )
set_target_properties( ${M1D_APP_NAME} PROPERTIES FOLDER "${BASE_FOLDER}/${M1D_APP_NAME}"
                                                  SUFFIX ".exe" )

create_application_run_script( ${M1D_APP_NAME} )

application_env_var( TARGET ${M1D_APP_NAME} VARS
     "CTCDIR"      "$MISCDIR"
)

include_directories(
    ${M1D_LIB_INCLUDE_DIRS}
)

include_directories( SYSTEM
    ${HDF5_INCLUDE_DIRS}
)

target_link_libraries( ${M1D_APP_NAME}
              modelpseudo1dlib
              DataAccess
              DataMining
              cmbapi
              FileSystem
              TableIO
              utilities
              ${HDF5_LIBRARIES}
)

if (WIN32)
  # Windows.h defines 'min' and 'max' macros if NOMINMAX is not defined. These
  # macros collide with std::min and std::max
  add_definitions(-DNOMINMAX)
endif()

install( TARGETS ${M1D_APP_NAME}
    RUNTIME DESTINATION bin 
  )
