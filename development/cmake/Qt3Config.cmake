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
   
find_package(Qt3 3.3.6)
include(cmake/Qt3Macros.cmake)

add_external_package_info(
   CAPABILITY QT3
   NAME    "Qt3"
   VENDOR  "Digia"
   VERSION "3.3.6"
   LICENSE_TYPE "Commercial"
   LICENSE_FILE ""
   URL "http://qt-project.org"
   DESCRIPTION "A cross-platform application and UI framework"
   DEPENDENCY "Runtime"
)

