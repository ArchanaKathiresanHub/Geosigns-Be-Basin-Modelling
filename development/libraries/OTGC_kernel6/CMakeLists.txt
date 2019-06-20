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

# This library depends on genex6 for some include files only
set( LIB_NAME "OTGC_kernel6" )
create_bm_library( TARGET ${LIB_NAME}
                   LIBRARIES
                       genex6
                       utilities
                       EosPack
                       CBMGenerics
                   INSTALLTARGET)

add_definitions(-DUSEOTGC)
