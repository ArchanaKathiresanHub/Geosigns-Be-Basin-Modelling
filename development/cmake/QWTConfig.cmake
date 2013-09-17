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

set(QWT_FOUND TRUE)
set(QWT_ROOT "/nfs/rvl/users/ibs/IBS/EXTERNALS/QWT/curr/LinuxRHEL64_x86_64/64/OPTIMIZED" CACHE PATH "Installation directory of QT Widgets libraries")
set(QWT_INCLUDE_DIRS "${QWT_ROOT}/include")
set(QWT_LIBRARIES "${QWT_ROOT}/lib/libqwt.a")

add_external_package_info(
   CAPABILITY QWT
   NAME    "QWT"
   VENDOR  "Uwe Rathmann, Josef Wilgen"
   VERSION "4.2.0"
   LICENSE_TYPE "QWT v1"
   LICENSE_FILE "/nfs/rvl/users/ibs/IBS/EXTERNALS/QWT/4.2.0/SOURCE/COPYING"
   URL "http://qwt.sf.net"
   DESCRIPTION "Qt Widgets for Technical Applications"
   DEPENDENCY "Runtime"
)

