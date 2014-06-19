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

find_package(OpenMP)   

if (UNIX AND (NOT BUILD_SHARED_LIBS) )   
   set(OpenMP_LINK_FLAGS "-openmp-link static" CACHE STRING "Extra compiler flags to link with OpenMP")
endif()
# There is no need to include 3rdparty information, because in our case it is
# a compiler extension

