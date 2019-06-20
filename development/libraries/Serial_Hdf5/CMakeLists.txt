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

include_directories( SYSTEM ${HDF5_INCLUDE_DIRS} )

create_bm_library(TARGET "Serial_Hdf5"
                  LIBRARIES ${HDF5_LIBRARIES}
                  INSTALLTARGET)
