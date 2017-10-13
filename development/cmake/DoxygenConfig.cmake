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

set(DOXYGEN_EXECUTABLE "DOXYGEN_EXECUTABLE-NOTFOUND" CACHE PATH "Path to doxygen executable")
set(DOT_EXECUTABLE "DOT_EXECUTABLE-NOTFOUND" CACHE PATH "Path to dot executable")

find_package(Doxygen)

if(DOXYGEN_FOUND)
   add_custom_target(doc COMMENT "Generating Doxygen documentation")
   set_target_properties( doc PROPERTIES FOLDER "Documentation" )

   add_external_package_info(
      CAPABILITY Boost
      NAME    "Doxygen"
      VENDOR  "Dimitri van Heesch"
      VERSION "${DOXYGEN_VERSION}"
      LICENSE_TYPE "GPL"
      LICENSE_FILE "${THIRD_PARTY_DIR}/licenses/gpl-2.0.txt"
      URL "http://www.doxygen.org"
      DESCRIPTION "Doxygen is the defacto standard tool for generating documentation from annotated C++ sources"
      REQUIRED_AT  "Build"
      COUNTRY_OF_ORIGIN "The Netherlands"
      SHIPPED      "No"
      INCLUSION_TYPE "NA"
      USEABLE_STAND_ALONE "No"
      CONTAINS_CRYPTO "Unknown"
      ECCN         "Unknown"
   )
endif()

