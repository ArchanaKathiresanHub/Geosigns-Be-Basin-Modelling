#########################################################################
#                                                                       #
# Copyright (C) 2012-2014 Shell International Exploration & Production. #
# All rights reserved.                                                  #
#                                                                       #
# Developed under license for Shell by PDS BV.                          #
#                                                                       #
# Confidential and proprietary source code of Shell.                    #
# Do not distribute without written permission from Shell.              #
#                                                                       #
#########################################################################

if (ParaView_FOUND)
   include_directories(	${PROJECT_SOURCE_DIR}/libraries/DataModel/src src ${HDF5_INCLUDE_DIRS})
   bm_include_libraries(DataAccess TableIO)

   add_paraview_plugin(ParaViewReader "1.0" 
          SERVER_MANAGER_XML ParaViewReader.xml
          SERVER_MANAGER_SOURCES src/ParaViewReader.C
          REQUIRED_ON_SERVER
      )

   target_link_libraries( ParaViewReader
         DataAccess
         TableIO
         SerialDataAccess
         ${HDF5_LIBRARIES}
   )

endif()
