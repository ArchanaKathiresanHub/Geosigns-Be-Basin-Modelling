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
include(cmake/EnvSetup.cmake)


set(INTEL_CXX_ROOT "/apps/3rdparty/intel/ics2013/composer_xe_2013.5.192" CACHE PATH "Path to Intel's compiler collection")

if (DEFINED ENV{CXX} )
    
   # Then just autodetect
   enable_language(CXX)

elseif(UNIX)

   #
   # On Shell Global Linux: Choose the Intel compiler
   #
   set(INTEL_COMPILER)

   # Generate compiler wrapper that loads environment
   set(BM_COMPILER_ENVIRONMENT_SETUP "${INTEL_CXX_ROOT}/bin/compilervars.sh intel64")
   set(COMPILER_TO_WRAP "icc")
   configure_file("cmake/compiler_wrap.sh.cmake" "${CMAKE_BINARY_DIR}/aux/cc_wrap.sh" @ONLY)

   set(COMPILER_TO_WRAP "icpc")
   configure_file("cmake/compiler_wrap.sh.cmake" "${CMAKE_BINARY_DIR}/aux/cxx_wrap.sh" @ONLY)

   file(COPY "${CMAKE_BINARY_DIR}/aux/cc_wrap.sh" "${CMAKE_BINARY_DIR}/aux/cxx_wrap.sh"
        DESTINATION "${CMAKE_BINARY_DIR}"
        FILE_PERMISSIONS
           OWNER_READ OWNER_WRITE OWNER_EXECUTE
           GROUP_READ GROUP_WRITE GROUP_EXECUTE
           WORLD_READ WORLD_EXECUTE
       )

   # Set compiler to wrapper
   set(CMAKE_C_COMPILER "${CMAKE_BINARY_DIR}/cc_wrap.sh" CACHE FILEPATH "C Compiler")
   set(CMAKE_CXX_COMPILER "${CMAKE_BINARY_DIR}/cxx_wrap.sh" CACHE FILEPATH "C++ Compiler")

   # Add environment set-up scripts to generated script
   add_environment_source_script(CSHELL "${INTEL_CXX_ROOT}/bin/compilervars.csh intel64")
   add_environment_source_script(BOURNE "${INTEL_CXX_ROOT}/bin/compilervars.sh intel64")

   # Add package info
   add_external_package_info(
       CAPABILITY   Compiler
       NAME         "Compiler"
       VENDOR       "Intel"
       VERSION      "13.1.3 20130607"
       LICENSE_TYPE "Commercial"
       LICENSE_FILE "${INTEL_CXX_ROOT}/Documentation/en_US/clicense"
       URL          "http://software.intel.com/en-us/intel-compilers"
       DESCRIPTION  "Intel Compilers"
       REQUIRED_AT  "Build"
       COUNTRY_OF_ORIGIN "USA"
       SHIPPED      "No"
       INCLUSION_TYPE "NA"
       USEABLE_STAND_ALONE "No"
       CONTAINS_CRYPTO "Unknown"
       ECCN         "Unknown"
  )

else()

   # First detect the Compiler
   enable_language(CXX)

   if (MSVC)
       #
       # On Windows: Use Microsoft Visual C++ compiler
       #
   add_external_package_info(
          CAPABILITY   Compiler
          NAME         "Visual C/C++"
          VENDOR       "Microsoft"
          VERSION      "${MSVC_VERSION}"
          LICENSE_TYPE "Commercial"
          LICENSE_FILE ""
          URL          "http://www.microsoft.com/visualstudio"
          DESCRIPTION  "Microsoft Visual Studio"
          REQUIRED_AT  "Build"
          COUNTRY_OF_ORIGIN "USA"
          SHIPPED      "No"
          INCLUSION_TYPE "NA"
          USEABLE_STAND_ALONE "No"
          CONTAINS_CRYPTO "Unknown"
          ECCN         "Unknown"
       )
   else()
      message(WARNING "Unknown compiler on unknown OS is used. It is not covered by the generated third party package list")
   endif()

endif()

