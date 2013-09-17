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

include( cmake/AddPackage.cmake)

# Use the Boost C++ library that is available on Red Hat EL 5.x 

find_package( Boost 1.33.0 )
if (Boost_FOUND)
   include_directories(${Boost_INCLUDE_DIRS})

   add_external_package_info(
      CAPABILITY BoostLib
      NAME    "Boost"
      VENDOR  "Boost"
      VERSION "1.33.0"
      LICENSE_TYPE "Boost v1"
      LICENSE_FILE "${THIRD_PARTY_DIR}/licenses/Boost-1.33.0.txt"
      URL "http://boost.org"
      DESCRIPTION "Free peer-reviewed portable C++ source libraries"
      DEPENDENCY "Runtime"
   )
endif()

