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

include(cmake/AddPackage.cmake)

# Include SUM++ library. Shell library for SA/UA 

set( SUMPP_ROOT "${THIRD_PARTY_DIR}/SUMlib" CACHE PATH "Path to SUM++ sources" )
set( SUMPP_INCLUDE_DIR "${SUMPP_ROOT}/include")

set( SUMPP_FOUND TRUE )

add_subdirectory(${THIRD_PARTY_DIR}/SUMlib SUMlib)

add_external_package_info(
       CAPABILITY SUMlib
       NAME     "Stochastic Uncertainty Manager. SUM++"
       VENDOR   "Shell"
       VERSION  "18.02.2014"
       LICENSE_TYPE "Shell propriety package. Confidential"
       LICENSE_FILE "${THIRD_PARTY_DIR}/SUMlib/COPYING"
       URL      ""
       DESCRIPTION "Stochastic Uncertainty Manager library"
       REQUIRED_AT  "Runtime"
       COUNTRY_OF_ORIGIN "EU"
       SHIPPED      "Yes"
       INCLUSION_TYPE "Static Link"
       USEABLE_STAND_ALONE "No"
       CONTAINS_CRYPTO "No"
       ECCN         "Unknown"
)
