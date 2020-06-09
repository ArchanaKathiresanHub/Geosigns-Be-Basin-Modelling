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
   include_directories(	src )
   include_directories(	SYSTEM ${HDF5_INCLUDE_DIRS} )
   add_paraview_plugin(ParaViewReader "1.0" 
          SERVER_MANAGER_XML ParaViewReader.xml
          SERVER_MANAGER_SOURCES src/ParaViewReader.cpp
          REQUIRED_ON_SERVER
      )

   target_link_libraries( ParaViewReader
         SerialDataAccess
         ${HDF5_LIBRARIES}
   )

endif()
