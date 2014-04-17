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


if (UNIX)
# On the Unix platform use the Boost C++ library that is available on Red Hat EL 5.x 
   find_package( Boost 1.33.0 REQUIRED)
   
   add_external_package_info(
      CAPABILITY BoostLib
      NAME    "Boost"
      VENDOR  "Boost"
      VERSION "1.33.0"
      LICENSE_TYPE "Boost v1"
      LICENSE_FILE "${THIRD_PARTY_DIR}/licenses/Boost-1.33.0.txt"
      URL "http://boost.org"
      DESCRIPTION "Free peer-reviewed portable C++ source libraries"
      REQUIRED_AT  "Runtime"
      COUNTRY_OF_ORIGIN ""
      SHIPPED      "Yes"
      INCLUSION_TYPE "Static Link"
      USEABLE_STAND_ALONE "No"
      CONTAINS_CRYPTO "No"
      ECCN         "EAR99"
      ECCN_EVIDENCE "https://sps.sede-coe.pds.nl/CoE-II/Basin Modeling/Release documents/Export Classification Tracking/2013.10-ECCN_Discussion_with_George_Jolly.msg"
   )   
   
elseif(WIN32)

    set(BOOST_ROOT "c:/opt/boost_1_55_0")
	find_package( Boost 1.55.0  REQUIRED )
   
    add_external_package_info(
      CAPABILITY BoostLib
      NAME    "Boost"
      VENDOR  "Boost"
      VERSION "1.55.0"
      LICENSE_TYPE "Boost v1"
      LICENSE_FILE "${THIRD_PARTY_DIR}/licenses/Boost-1.55.0.txt"
      URL "http://boost.org"
      DESCRIPTION "Free peer-reviewed portable C++ source libraries"
      REQUIRED_AT  "Runtime"
      COUNTRY_OF_ORIGIN ""
      SHIPPED      "Yes"
      INCLUSION_TYPE "Static Link"
      USEABLE_STAND_ALONE "No"
      CONTAINS_CRYPTO "No"
      ECCN         "EAR99"
      ECCN_EVIDENCE "https://sps.sede-coe.pds.nl/CoE-II/Basin Modeling/Release documents/Export Classification Tracking/2013.10-ECCN_Discussion_with_George_Jolly.msg"
   )
else()
    message(ERROR "OS not supported")
endif()


include_directories(${Boost_INCLUDE_DIRS})




