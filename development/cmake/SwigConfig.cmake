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

include(files/AddPackage.cmake)

set(SWIG_FOUND TRUE CACHE "Swig is found")
set(SWIG_DIR "${THIRD_PARTY_DIR}/swigwin-2.0.0" CACHE "The directory where SWIG is installed")
set(SWIG_EXECUTABLE "${SWIG_DIR}/swig.exe" CACHE "The path to the SWIG executable")
set(SWIG_VERSION    "2.0.0" CACHE "The version number of the swig executable")

add_external_package_info( 
       CAPABILITY Swig
       NAME     "Swig"
       VENDOR   ""
       VERSION  "${SWIG_VERSION}"
       LICENSE_TYPE "GPL v3"
       LICENSE_FILE "${SWIG_DIR}/LICENSE"
       URL      "http://www.swig.org/"
       DESCRIPTION "Connects C/C++/Objective C to some high-level programming languages."
       DEPENDENCY "Build"
)                   

include(UseSWIG)
