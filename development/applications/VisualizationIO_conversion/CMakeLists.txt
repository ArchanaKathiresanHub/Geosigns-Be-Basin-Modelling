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
source_group("Header Files" FILES ${all_headers})

FILE( GLOB all_srcs src/*.cpp)
source_group("Source Files" FILES ${all_srcs})

set( TARGET_NAME "VisualizationIO_convert" )
add_executable( ${TARGET_NAME} ${all_srcs} ${all_headers} )
set_target_properties( ${TARGET_NAME} PROPERTIES SUFFIX ".exe" )
set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER "${BASE_FOLDER}/${TARGET_NAME}" )
create_application_run_script( ${TARGET_NAME} )

application_env_var( TARGET ${TARGET_NAME} VARS
   "CTCDIR" "$MISCDIR"
)

include_directories( SYSTEM
   ${HDF5_INCLUDE_DIRS}
   ${PUGIXML_INCLUDE_DIR}
   ${LZ_INCLUDE_DIR}
)

bm_include_libraries(
   DataModel
   DataAccess
   SerialDataAccess
   DataAccess
   TableIO
   FileSystem
   utilities
   VisualizationIO
   VisualizationIO_projectHandle
)

target_link_libraries(
   ${TARGET_NAME}
   DataModel
   DataAccess
   SerialDataAccess
   DataAccess
   TableIO
   FileSystem
   VisualizationIO
   VisualizationIO_projectHandle
   PugiXMLlib
   LZlib
   ${HDF5_LIBRARIES}
   ${Boost_LIBRARIES}
)

install(TARGETS ${TARGET_NAME}
   RUNTIME DESTINATION bin
)

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
