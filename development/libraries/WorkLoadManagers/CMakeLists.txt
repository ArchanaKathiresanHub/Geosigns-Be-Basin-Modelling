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

set( LIB_NAME "WorkLoadManagers")

include_directories( SYSTEM )

create_bm_library( TARGET ${LIB_NAME} 
	LIBRARIES ${Boost_LIBRARIES}
)

add_gtest( NAME "${LIB_NAME}::TestWLMScripts"
	SOURCES test/SDUWorkLoadManagerTest.cpp 
	LIBRARIES WorkLoadManagers ${Boost_LIBRARIES}
	FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)