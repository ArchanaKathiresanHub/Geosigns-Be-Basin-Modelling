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
FILE( GLOB all_headers src/*.h )
FILE( GLOB all_sources src/*.C )

source_group(include FILES ${all_headers})
source_group(source  FILES ${all_sources})

add_library(DataModel
	${all_sources}
	${all_headers}
)
