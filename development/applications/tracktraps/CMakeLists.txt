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


FILE( GLOB all_headers src/*.h )
source_group(include FILES ${all_headers})

FILE( GLOB all_srcs src/*.cpp)
source_group(source FILES ${all_srcs})

set( APP_NAME "tracktraps" )
add_executable( ${APP_NAME}
   ${all_srcs}
   ${all_headers}
)

set_target_properties( ${APP_NAME} PROPERTIES SUFFIX ".exe" )
set_target_properties( ${APP_NAME} PROPERTIES FOLDER "${BASE_FOLDER}/${APP_NAME}" )

create_application_run_script( ${APP_NAME} )

application_env_var( TARGET ${APP_NAME} VARS
     "EOSPACKDIR"  "$MISCDIR/eospack"
)

include_directories( SYSTEM
   ${HDF5_INCLUDE_DIRS}
)


target_link_libraries(${APP_NAME}
   EosPack
   DataAccess
   SerialDataAccess
   TableIO
   utilities
   CBMGenerics
   ${HDF5_LIBRARIES}
)

install(TARGETS ${APP_NAME}
   RUNTIME DESTINATION bin
)

set(CFGFLS "${PROJECT_SOURCE_DIR}/geocase/misc")

configure_file(test/testData/Project.project3d ${CMAKE_CURRENT_BINARY_DIR}/Project.project3d COPYONLY)
configure_file(test/testData/Inputs.HDF ${CMAKE_CURRENT_BINARY_DIR}/Inputs.HDF COPYONLY)

add_gtest( NAME ${APP_NAME}::TrapperProperties
   SOURCES test/TrackTrapTest.cpp src/TrackTrap.cpp src/generics.cpp src/PersistentTrap.cpp src/PersistentTrapFactory.cpp src/TrackReservoir.cpp src/TrackProjectHandle.cpp
   LIBRARIES DataAccess EosPack TableIO SerialDataAccess utilities CBMGenerics ${HDF5_LIBRARIES}
   ENV_VARS EOSPACKDIR=${CFGFLS}/eospack GENEX5DIR=${CFGFLS}/genex50 CTCDIR=${CFGFLS}/
   FOLDER "${BASE_FOLDER}/${APP_NAME}"
)
