#########################################################################
#                                                                       #
# Copyright (C) 2012-2017 Shell International Exploration & Production. #
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

if (UNIX)

   add_external_project_to_repository(
         NAME TSLIB
         VERSION ${TSLIB_VERSION}
         ARCHIVE "${THIRD_PARTY_DIR}/sources/geocosm.tar.gz"
         ARCHIVE_MD5 "26daf94d85aa65a3a65a22c1272b51fa"
         PATCH_COMMAND     "${CMAKE_COMMAND}" "-E" "copy_directory" "." "{ROOT}"
         CONFIGURE_COMMAND "${CMAKE_COMMAND}" "-E" "echo" "TSLIB does not require configuration."
         BUILD_COMMAND     "${CMAKE_COMMAND}" "-E" "echo" "TSLIB does not require build."
         TEST_COMMAND      "ls" "bin/x86_64_linux/Release/libgeocosmxmllibbasecpp.so" "bin/x86_64_linux/Release/libgeocosmexception.so"
         INSTALL_COMMAND   "${CMAKE_COMMAND}" "-E" "echo" "TSLIB does not require install."
         CONFIGURE_OPTIONS 
           COMPILER "{CurrentCompiler}"  "--with-cc={CC}" "--with-cxx={CXX}"
           MPI      "{CurrentMPI}"
           SPEED    "Release"
           SPEED    "Debug"
           SPEED    "DebugAll"
           SPEED    "MemCheck"
           SPEED    "CodeCoverage"
           OS       "{CurrentPlatform}"
           LINK     "Dynamic"
           LINK     "Static"
         YIELD_LIBRARIES "tslib"
   )
   
   # TSLIB
   set( TSLIB_ROOT "${TSLIB_ROOT}" )
   set( TSLIB_LIBRARY_DIR "${TSLIB_ROOT}/bin/x86_64_linux/Release" CACHE PATH "Path to Geocosm's TsLib library directory" )
   set( TSLIB_INCLUDE_DIRS "${TSLIB_ROOT}/include;${TSLIB_ROOT}/geocosmxml" )
   set( TSLIB_LIBRARIES )
   list(APPEND TSLIB_LIBRARIES "${TSLIB_LIBRARY_DIR}/libgeocosmxmllibbasecpp.so" "${TSLIB_LIBRARY_DIR}/libgeocosmexception.so" )
   
   # Xerces
   set( XERCES_ROOT "${TSLIB_ROOT}/3rdparty/Xerces/xerces-c-3.1.0-x86_64-linux-gcc-3.4" CACHE PATH "Path to Xerces-C library" )
   set( XERCES_INCLUDE_DIRS "${XERCES_ROOT}/include" )
   set( XERCES_LIBRARIES "${XERCES_ROOT}/lib/libxerces-c-3.1.so" )
   
   # XSD
   set( XSD_ROOT "${TSLIB_ROOT}/3rdparty/xsd/xsd-3.3.0-x86_64-linux-gnu/libxsd" CACHE PATH "Path to Codesynthesis's XSD library" )
   set( XSD_INCLUDE_DIRS "${XSD_ROOT}" )

elseif (WIN32) # windows

   message(STATUS "Geocosm's Touchstone not supported yet on Windows")
   
endif ()

# -------------
# Packages info
# -------------
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

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
