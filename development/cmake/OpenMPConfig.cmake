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

if (UNIX)   
   set(OpenMP_CXX_FLAGS "${OpenMP_CXX_FLAGS} -openmp-link static")
endif()
# There is no need to include 3rdparty information, because in our case it is
# a compiler extension

