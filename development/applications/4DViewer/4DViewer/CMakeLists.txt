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

FILE( GLOB all_ui *.ui)
source_group(ui FILES ${all_ui})

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package(Qt4 4.8.4 REQUIRED QtGui QtCore)

set(OIVHOME $ENV{OIVHOME})
set(OIVARCH $ENV{OIVARCH})

set(OIV_INCLUDE_DIR ${OIVHOME}/include)
set(OIV_LIB_DIR_REL ${OIVHOME}/arch-Windows-x86_64-msvc12-Release/lib)
set(OIV_LIB_DIR_DBG ${OIVHOME}/arch-Windows-x86_64-msvc12-Debug/lib)

link_directories( ${OIV_LIB_DIR_REL} ${OIV_LIB_DIR_DBG})

add_executable( 4DViewer ${all_srcs} ${all_headers} ${all_ui})

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

target_link_libraries(4DViewer
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
   Qt4::QtGui
   Qt4::QtCore
)

install( TARGETS 4DViewer
   DESTINATION bin
)
