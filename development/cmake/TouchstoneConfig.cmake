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

#
# Add Geocosm's Touchstone libraries + all its 3rd party components
#

set( TSLIB_ROOT "TSLIB-NOTFOUND" CACHE PATH "Path to Geocosm's TsLib" )
set( TSLIB_LIBRARY_DIR "TSLIB_LIBRARY_DIR-NOTFOUND" CACHE PATH "Path to Geocosm's TsLib library dir" )
set( TSLIB_INCLUDE_DIRS "${TSLIB_ROOT}/include;${TSLIB_ROOT}/geocosmxml" )
find_library( TSLIB_GEOCOSMBASECPP "geocosmxmllibbasecpp" PATHS "${TSLIB_LIBRARY_DIR}" NO_DEFAULT_PATH)
find_library( TSLIB_GEOCOSMEXCEPTION "geocosmexception"   PATHS "${TSLIB_LIBRARY_DIR}" NO_DEFAULT_PATH)
set( TSLIB_LIBRARIES )
list(APPEND TSLIB_LIBRARIES "${TSLIB_GEOCOSMBASECPP}" "${TSLIB_GEOCOSMEXCEPTION}")
mark_as_advanced( TSLIB_GEOCOSMBASECPP TSLIB_GEOCOSMEXCEPTION )

add_external_package_info(
      CAPABILITY TsLib
      NAME    "TsLib"
      VENDOR  "Geocosm"
      VERSION "${TSLIB_VERSION}"
      LICENSE_TYPE "Commercial"
      LICENSE_FILE ""
      URL "http://www.geocosm.net/consortia/touchstone.htm"
      DESCRIPTION "Touchstone Resq library: software system for analysis of controls on reservoir quality in analog sandstones and for forward modeling of sandstone diagenesis and petrophysical properties"
      REQUIRED_AT  "Runtime"
      COUNTRY_OF_ORIGIN "USA"
      SHIPPED      "Yes"
      INCLUSION_TYPE "Static Link"
      USEABLE_STAND_ALONE "No"
      CONTAINS_CRYPTO "No"
      ECCN         "EAR99"
)

set( XERCES_ROOT "XERCES-NOTFOUND" CACHE PATH "Path to Xerces-C library" )
set( XERCES_INCLUDE_DIRS "${XERCES_ROOT}/include")
find_library( XERCES_LIBRARIES "xerces-c-3.1" PATHS "${XERCES_ROOT}/lib" NO_DEFAULT_PATH)
add_external_package_info(
      CAPABILITY XercesC
      NAME    "Xerces-C"
      VENDOR  "Apache"
      VERSION "3.1.0"
      LICENSE_TYPE "Apache v2"
      LICENSE_FILE "${THIRD_PARTY_DIR}/licenses/XercesC-3.1.0.txt"
      URL "http://xerces.apache.org/xerces-c/"
      DESCRIPTION "A validating XML parser written in a portable subset of C++"
      REQUIRED_AT  "Runtime"
      REQUIRED_BY "TsLib7"
      COUNTRY_OF_ORIGIN ""
      SHIPPED      "Yes"
      INCLUSION_TYPE "Dynamic Linked AND Separate Executable"
      USEABLE_STAND_ALONE "Yes"
      CONTAINS_CRYPTO "No"
      ECCN         "EAR99"
)

set( XSD_ROOT "XSD-NOTFOUND" CACHE PATH "Path to Codesynthesis's XSD library")
set( XSD_INCLUDE_DIRS "${XSD_ROOT}")
add_external_package_info(
      CAPABILITY XSD
      NAME    "XSD"
      VENDOR  "Code Synthesis"
      VERSION "3.3.0b3"
      LICENSE_TYPE "Commercial"
      LICENSE_FILE ""
      URL "http://www.codesynthesis.com/products/xsd/"
      DESCRIPTION "W3C XML Schema to C++ data binding compiler"
      REQUIRED_AT  "Runtime"
      REQUIRED_BY "TsLib7"
      COUNTRY_OF_ORIGIN "South Africa"
      SHIPPED        "Yes"
      INCLUSION_TYPE "Dynamic Linked AND Separate Executable"
      USEABLE_STAND_ALONE "Yes"
      CONTAINS_CRYPTO "No"
      ECCN         "Unknown"
)

set( MCR_ROOT "MCR-NOTFOUND" CACHE PATH "Path to Matlab ")
set( MCR_VERSION "Unknown" CACHE STRING "Matlab version")
add_external_package_info(
      CAPABILITY Matlab
      NAME       "Matlab"
      VENDOR     "The Mathworks, Inc."
      VERSION    "${MCR_VERSION}"
      LICENSE_TYPE "Commercial"
      LICENSE_FILE  "${MCR_ROOT}/license.txt"
      URL        "http://www.mathworks.co.uk/products/matlab"
      DESCRIPTION "High-level language and interactive environment for numerical computation, visualization, and programming"
      REQUIRED_BY "TsLib7"
      REQUIRED_AT  "Runtime"
      COUNTRY_OF_ORIGIN "USA"
      SHIPPED        "No"
      INCLUSION_TYPE "Separate Executable"
      USEABLE_STAND_ALONE "Yes"
      CONTAINS_CRYPTO "No"
)
	
add_external_package_info(
      CAPABILITY Matlab_JRE
      NAME       "Java Runtime Environment"
      VENDOR     "Oracle"
      VERSION    "6"
      LICENSE_TYPE "Commercial"
      LICENSE_FILE "${MCR_ROOT}/sys/java/jre/glnxa64/jre/LICENSE"
      URL "http://www.oracle.com/technetwork/java/javase/overview/index.html"
      DESCRIPTION "Java Runtime Environment used by Matlab that is used by Geocosm's TsLib"
      REQUIRED_BY "Matlab"
      REQUIRED_AT  "Runtime"
      COUNTRY_OF_ORIGIN "USA"
      SHIPPED        "No"
      INCLUSION_TYPE "Separate Executable"
      USEABLE_STAND_ALONE "Yes"
      CONTAINS_CRYPTO "Yes"
)  



