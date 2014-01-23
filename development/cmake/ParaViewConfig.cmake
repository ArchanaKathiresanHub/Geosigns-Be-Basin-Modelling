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

include( ${PROJECT_SOURCE_DIR}/cmake/AddPackage.cmake)

find_package(ParaViewInstallation CONFIG
      CONFIGS "ParaViewConfig.cmake"
      PATHS "${CBM_HOME}/Tools/paraview/Paraview-4.1.0-Release/Linux64x_26/lib/cmake/paraview-4.1"
      NO_DEFAULT_PATH
)

add_external_package_info(
   CAPABILITY ParaView
   NAME    "ParaView (INCOMPLETE INFO)"
   VENDOR  "Kitware"
   VERSION "4.1.0"
   LICENSE_TYPE "BSD"
   LICENSE_FILE "${THIRD_PARTY_DIR}/licenses/ParaView-4.1.0.txt"
   URL "http://paraview.org"
   DESCRIPTION "an open-source, multi-platform data analysis and visualization application."
   REQUIRED_AT  "Runtime"
   COUNTRY_OF_ORIGIN ""
   SHIPPED      "Undertermined"
   INCLUSION_TYPE "Undetermined"
   USEABLE_STAND_ALONE "No"
   CONTAINS_CRYPTO "No"
   ECCN         "Unknown"
)

