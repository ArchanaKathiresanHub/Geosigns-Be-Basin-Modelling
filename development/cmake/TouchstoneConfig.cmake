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

set( TS_ROOT "/glb/home/ksaho3/TsLib3.1.1_64" CACHE PATH "Location of Geocosm's Touchstone library")
set( TS_INCLUDE_DIRS "${TS_ROOT}/Lib" )
set( TS_LIBRARIES "${TS_ROOT}/Lib/libTsLib.a" )

add_external_package_info(
      CAPABILITY TsLib3
      NAME    "TsLib"
      VENDOR  "Geocosm"
      VERSION "3.1.1"
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
 
set( TS7_ROOT "/nfs/rvl/groups/ept-sg/SWEast/Cauldron/ksaho3/opt/tslib3.2" CACHE PATH "Path to Geocosm's TsLib 7.0" )
set( TS7_LIBRARY_DIR "${TS7_ROOT}/bin/x86_64_linux/Release")
set( TS7_INCLUDE_DIRS "${TS7_ROOT}/include;${TS7_ROOT}/geocosmxml" )
set( TS7_LIBRARIES)
foreach (LIB geocosmxmllibbasecpp geocosmexception )
   list(APPEND TS7_LIBRARIES "${TS7_LIBRARY_DIR}/lib${LIB}.so")
endforeach()
     
add_external_package_info(
      CAPABILITY TsLib7
      NAME    "TsLib"
      VENDOR  "Geocosm"
      VERSION "7.3.0.3436"
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
 
set( XERCES_ROOT "${TS7_ROOT}/3rdparty/Xerces/xerces-c-3.1.0-x86_64-linux-gcc-3.4" CACHE PATH "Path to Xerces-C library" )
set( XERCES_INCLUDE_DIRS "${XERCES_ROOT}/include")
set( XERCES_LIBRARIES "${XERCES_ROOT}/lib/libxerces-c-3.1.so")


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

set( XSD_ROOT "${TS7_ROOT}/3rdparty/xsd/xsd-3.3.0.b3-i686-linux-gnu/libxsd" CACHE PATH "Path to Codesynthesis's XSD library")
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

add_external_package_info(
      CAPABILITY MatlabCompilerRuntime
      NAME       "Matlab Compiler Runtime"
      VENDOR     "The Mathworks, Inc."
      VERSION    "7.13"
      LICENSE_TYPE "Commercial"
      LICENSE_FILE  "${TS7_ROOT}/3rdparty/matlabmcr/matlab/v713/license.txt"
      URL        "http://www.mathworks.co.uk/products/compiler/mcr"
      DESCRIPTION "High-level language and interactive environment for numerical computation, visualization, and programming"
      REQUIRED_BY "TsLib7"
      REQUIRED_AT  "Runtime"
      COUNTRY_OF_ORIGIN "USA"
      SHIPPED        "Yes"
      INCLUSION_TYPE "Separate Executable"
      USEABLE_STAND_ALONE "Yes"
      CONTAINS_CRYPTO "No"
      ECCN         "5D992"
      ECCN_EVIDENCE "https://sps.sede-coe.pds.nl/CoE-II/Basin Modeling/Release documents/Export Classification Tracking/2013.10-ECCN_Discussion_with_George_Jolly.msg"
)
	
add_external_package_info(
      CAPABILITY MatlabCompilerRuntime_InstallerJRE
      NAME       "Java Runtime Environment"
      VENDOR     "Oracle"
      VERSION    "1.5.0_04"
      LICENSE_TYPE "Commercial"
      LICENSE_FILE  "${TS7_ROOT}/3rdparty/matlabmcr/matlab/_jvm/LICENSE"
      URL "http://www.oracle.com/technetwork/java/javase/overview/index.html"
      DESCRIPTION "Java Runtime Environment used by InstallShield installer of Matlab Compiler Runtime that is used by Geocosm's TsLib"
      REQUIRED_BY "MatlabCompilerRuntime"
      REQUIRED_AT  "Build"
      COUNTRY_OF_ORIGIN "USA"
      SHIPPED        "Yes"
      INCLUSION_TYPE "Separate Executable"
      USEABLE_STAND_ALONE "Yes"
      CONTAINS_CRYPTO "Yes"
      ECCN         "5D992"
      ECCN_EVIDENCE "https://sps.sede-coe.pds.nl/CoE-II/Basin Modeling/Release documents/Export Classification Tracking/2013.10-ECCN_Discussion_with_George_Jolly.msg"
      ECCN_EVIDENCE "http://www.oracle.com/us/products/export/eccn-matrix-software-412042.pdf"
)

add_external_package_info(
      CAPABILITY MatlabCompilerRuntime_JRE
      NAME       "Java Runtime Environment"
      VENDOR     "Oracle"
      VERSION    "1.6.0_12"
      LICENSE_TYPE "Commercial"
      LICENSE_FILE "${TS7_ROOT}/3rdparty/matlabmcr/matlab/v713/sys/java/jre/glnxa64/jre/LICENSE"
      URL "http://www.oracle.com/technetwork/java/javase/overview/index.html"
      DESCRIPTION "Java Runtime Environment used by Matlab Compiler Runtime that is used by Geocosm's TsLib"
      REQUIRED_BY "MatlabCompilerRuntime"
      REQUIRED_AT  "Runtime"
      COUNTRY_OF_ORIGIN "USA"
      SHIPPED        "Yes"
      INCLUSION_TYPE "Separate Executable"
      USEABLE_STAND_ALONE "Yes"
      CONTAINS_CRYPTO "Yes"
      ECCN         "5D992"
      ECCN_EVIDENCE "https://sps.sede-coe.pds.nl/CoE-II/Basin Modeling/Release documents/Export Classification Tracking/2013.10-ECCN_Discussion_with_George_Jolly.msg"
      ECCN_EVIDENCE "http://www.oracle.com/us/products/export/eccn-matrix-software-412042.pdf"
)  

