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
   include_directories(src)
   add_paraview_plugin(ParaViewReader "1.0" 
          SERVER_MANAGER_XML ParaViewReader.xml
          SERVER_MANAGER_SOURCES src/ParaViewReader.C
          REQUIRED_ON_SERVER
      )
endif()
