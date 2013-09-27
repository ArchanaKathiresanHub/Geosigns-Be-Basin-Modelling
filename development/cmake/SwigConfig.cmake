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

set(SWIG_FOUND TRUE)
set(SWIG_DIR "${THIRD_PARTY_DIR}/swigwin-2.0.0" CACHE PATH "The directory where SWIG is installed")
set(SWIG_EXECUTABLE "${SWIG_DIR}/swig.exe" CACHE FILEPATH "The path to the SWIG executable")
set(SWIG_VERSION    "2.0.0" CACHE STRING "The version number of the swig executable")

add_external_package_info( 
       CAPABILITY Swig
       NAME     "Swig"
       VENDOR   "Swig"
       VERSION  "${SWIG_VERSION}"
       LICENSE_TYPE "GPL v3"
       LICENSE_FILE "${SWIG_DIR}/LICENSE"
       URL      "http://www.swig.org/"
       DESCRIPTION "Connects C/C++/Objective C to some high-level programming languages."
       REQUIRED_AT  "Build"
       COUNTRY_OF_ORIGIN "USA"
       SHIPPED      "No"
       INCLUSION_TYPE "NA"
       USEABLE_STAND_ALONE "No"
       CONTAINS_CRYPTO "No"
       ECCN         "Unknown"
)                   

include(UseSWIG)
