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
FILE( GLOB all_headers *.h )
source_group(include FILES ${all_headers})

FILE( GLOB all_srcs *.cpp)
source_group(source FILES ${all_srcs})

link_directories( ${OIV_LIB_DIR_REL} ${OIV_LIB_DIR_DBG})

add_executable( 4DRenderServer ${all_srcs} ${all_headers})

set(OIV_LIBRARIES RemoteVizServices MeshVizDataMapping MeshVizExtractor MeshViz VolumeViz LDM Inventor InventorGL InventorBase)

include_directories(
   ${HDF5_INCLUDE_DIRS}
   ${OIV_INCLUDE_DIR}
)

bm_include_libraries(
   TableIO
   DataAccess
   DataModel
   VisualizationIO   
   Visualization4D
)

target_link_libraries(4DRenderServer
   VisualizationIO
   Visualization4D
   CBMGenerics
   Serial_Hdf5
   DataAccess
   SerialDataAccess
   TableIO
   utilities
   ${HDF5_LIBRARIES}
   ${Boost_LIBRARIES}
   ${OIV_LIBRARIES}
)

install( TARGETS 4DRenderServer
   DESTINATION bin
)

