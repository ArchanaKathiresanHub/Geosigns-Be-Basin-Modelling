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

set(suggestedOpenMPLinkFlags)
if (BM_USE_INTEL_COMPILER AND NOT BUILD_SHARED_LIBS)
  set(suggestedOpenMPLinkFlags "-openmp-link static")
endif()

set(OpenMP_LINK_FLAGS "${suggestedOpenMPLinkFlags}" CACHE STRING "Extra flags to link with OpenMP library")
