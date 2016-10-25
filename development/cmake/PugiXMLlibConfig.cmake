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

set( PUGIXML_ROOT "${THIRD_PARTY_DIR}/pugixml-1.7" CACHE PATH "Path to Pugi XML sources" )
set( PUGIXML_INCLUDE_DIR "${PUGIXML_ROOT}/include")

set( PUGIXML_FOUND TRUE )

add_subdirectory(${THIRD_PARTY_DIR}/pugixml-1.7 pugixml-1.7)

add_external_package_info(
       CAPABILITY PugiXML
       NAME     "Pugixml"
       VENDOR   ""
       VERSION  "1.7"
       LICENSE_TYPE "MIT"
       LICENSE_FILE "${THIRD_PARTY_DIR}/licenses/pugixml-1.7.txt"
       URL      "http://pugixml.org/"
       DESCRIPTION "Pugixml is a light-weight C++ XML processing library"
       REQUIRED_AT  "Runtime"
       COUNTRY_OF_ORIGIN "EU"
       SHIPPED      "Yes"
       INCLUSION_TYPE "Static Link"
       USEABLE_STAND_ALONE "No"
       CONTAINS_CRYPTO "No"
       ECCN         "Unknown"
)
