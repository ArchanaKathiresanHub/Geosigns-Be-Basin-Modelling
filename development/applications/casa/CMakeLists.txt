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

configure_file(../../libraries/casaAPI/src/CauldronEnvConfig.h.in CauldronEnvConfig.h)

set( APP_NAME "casa" )

add_executable( ${APP_NAME}
  ${all_srcs}
  ${all_headers}
)

set_target_properties( ${APP_NAME} PROPERTIES SUFFIX ".exe"
                                              FOLDER "${BASE_FOLDER}/${APP_NAME}" )

create_application_run_script( ${APP_NAME} )

application_env_var( TARGET ${APP_NAME} VARS
     "GENEXDIR"    "$MISCDIR/genex40"
     "GENEX5DIR"   "$MISCDIR/genex50"
     "GENEX6DIR"   "$MISCDIR/genex60"
     "OTGCDIR"     "$MISCDIR/OTGC"
     "CTCDIR"      "$MISCDIR"
     "EOSPACKDIR"  "$MISCDIR/eospack"
)
    
if (UNIX)
add_dependencies(${APP_NAME} Boost)
endif (UNIX)

bm_include_libraries(
   TableIO
   casaAPI
   cmbAPI
   CBMGenerics
   FileSystem
   DataAccess
   utilities
)

include_directories( utilities
   ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( SYSTEM
   ${SUMPP_INCLUDE_DIR}
   ${NNLIB_INCLUDE_DIR}
)

target_link_libraries( ${APP_NAME}
   casalib
   cmbapi
   CBMGenerics
   DataAccess
   SerialDataAccess
   Serial_Hdf5
   TableIO
   utilities
   SUMlib
   NNlib
   ${HDF5_LIBRARIES}
   ${Boost_LIBRARIES}
   ${LSF_LIBS}
)

install( TARGETS ${APP_NAME}
   DESTINATION bin
)
