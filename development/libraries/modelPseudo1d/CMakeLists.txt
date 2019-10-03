#########################################################################
#                                                                       #
# Copyright (C) 2012-2018 Shell International Exploration & Production. #
# All rights reserved.                                                  #
#                                                                       #
# Confidential and proprietary source code of Shell.                    #
# Do not distribute without written permission from Shell.              #
#                                                                       #
#########################################################################

set( M1D_LIB_NAME "modelpseudo1dlib" )

include_directories(
    src
)

include_directories( SYSTEM
    ${CMAKE_CURRENT_BINARY_DIR}
    ${HDF5_INCLUDE_DIRS}
)

create_bm_library( TARGET    ${M1D_LIB_NAME}
                   LIBRARIES DataAccess
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

set(CFGFLS "${PROJECT_SOURCE_DIR}/geocase/misc")

copy_test_file(Project.project3d)
copy_test_file(Input.HDF)
copy_test_file(Inputs.HDF)

add_gtest( NAME "ModelPseudo1dInputManager"
    SOURCES src/ModelPseudo1dInputManager.cpp test/TestModelPseudo1dInputManager.cpp
    LIBRARIES ${M1D_LIB_NAME} utilities
    FOLDER "${BASE_FOLDER}/${M1D_TARGET_NAME}" )

add_gtest( NAME "ModelPseudo1dProjectManager"
    SOURCES src/ModelPseudo1dProjectManager.cpp test/TestModelPseudo1dProjectManager.cpp
    LIBRARIES ${M1D_LIB_NAME} utilities cmbapi DataAccess DataMining TableIO
    ENV_VARS CTCDIR=${CFGFLS}
    FOLDER "${BASE_FOLDER}/${M1D_TARGET_NAME}" )
