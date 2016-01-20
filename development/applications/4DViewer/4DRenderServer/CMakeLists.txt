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

find_package(Qt4 4.8.4 REQUIRED QtGui QtCore)

set(OIVHOME $ENV{OIVHOME})
set(OIVARCH $ENV{OIVARCH})

set(OIV_INCLUDE_DIR ${OIVHOME}/include)
set(OIV_LIB_DIR ${OIVHOME}/${OIVARCH}/lib)

link_directories(${OIV_LIB_DIR})

add_executable( 4DRenderServer ${all_srcs} ${all_headers})

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
   DataAccess
   CBMGenerics
   Serial_Hdf5
   SerialDataAccess
   TableIO
   utilities
   VisualizationIO
   Visualization4D
   ${HDF5_LIBRARIES}
   ${Boost_LIBRARIES}
)

install( TARGETS 4DRenderServer
   DESTINATION bin
)

# target_link_libraries(4DRenderServer
# 	Visualization
# 	DataAccess
# 	VisualizationIO
# 	SerialDataAccess
# 	RemoteVizServices
# 	MeshVizDataMapping
# 	MeshVizExtractor
# 	MeshViz
# 	Inventor
# 	InventorGL
# 	InventorBase
# 	dl)

