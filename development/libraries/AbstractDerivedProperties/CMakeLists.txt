#########################################################################
#                                                                       #
# Copyright (C) 2018-2018 Shell International Exploration & Production. #
# All rights reserved.                                                  #
#                                                                       #
# Developed under license for Shell by PDS BV.                          #
#                                                                       #
# Confidential and proprietary source code of Shell.                    #
# Do not distribute without written permission from Shell.              #
#                                                                       #
#########################################################################

set( LIB_NAME "AbstractDerivedProperties" )
create_bm_library( TARGET ${LIB_NAME}
                   LIBRARIES DataModel utilities )

generate_dox( AbstractDerivedProperties.cfg )
