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

FILE( GLOB all_srcs src/*.C)
source_group(source FILES ${all_srcs})

set( LIB_NAME "Serial_Hdf5" )
add_library(${LIB_NAME} 
   ${all_srcs}
   ${all_headers}
)
set_target_properties( ${LIB_NAME} PROPERTIES FOLDER "${BASE_FOLDER}/${LIB_NAME}" )

include_directories( SYSTEM ${HDF5_INCLUDE_DIRS} )

target_link_libraries(${LIB_NAME} ${HDF5_LIBRARIES})

   
install(TARGETS ${LIB_NAME}
   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
