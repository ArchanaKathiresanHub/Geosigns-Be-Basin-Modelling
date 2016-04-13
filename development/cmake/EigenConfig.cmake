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

include( cmake/AddPackage.cmake)


# Add Boost as an external project
add_external_project_to_repository(
      NAME Eigen
      VERSION 3.2.5
      ARCHIVE "${THIRD_PARTY_DIR}/sources/eigen-3.2.5.tar.gz"
      ARCHIVE_MD5 "8cc513ac6ec687117acadddfcacf551b"
      CONFIGURE_COMMAND "${CMAKE_COMMAND}" "-E" "echo" "Eigen doesn't need to be configured."
      BUILD_COMMAND   "${CMAKE_COMMAND}" "-E" "echo" "Eigen doesn't need to be built."
      INSTALL_COMMAND "${CMAKE_COMMAND}" "-E" "copy_directory" "{SRCBUILD}/Eigen" "{ROOT}/include/eigen3/Eigen"
      CONFIGURE_OPTIONS 
        COMPILER "{CurrentCompiler}" 
        MPI      "{CurrentMPI}"
        SPEED    "Release"
        SPEED    "Debug"
        SPEED    "DebugAll"
        SPEED    "MemCheck"
        SPEED    "CodeCoverage"
        OS       "{CurrentPlatform}"
        LINK     "Dynamic"
        LINK     "Static"
)

set(EIGEN_INCLUDE_DIRS "${Eigen_ROOT}/include")

# Definining EIGEN_MPL2_ONLY is crucial for use within Shell, because this
# ensures that only codes is used with MPL2 and possible more permissive
# licenses (like BSD).
add_definitions(-DEIGEN_MPL2_ONLY)

add_external_package_info(
   CAPABILITY EigenLib
   NAME    "Eigen"
   VENDOR  "Eigen"
   VERSION "3.2"
   LICENSE_TYPE "MPL2"
   LICENSE_FILE "${THIRD_PARTY_DIR}/licenses/MPL2.txt"
   URL "http://eigen.tuxfamily.org"
   DESCRIPTION "C++ template library for linear algebra: matrices, vectors, numerical solvers, and related algorithms"
   REQUIRED_AT  "Runtime"
   COUNTRY_OF_ORIGIN "France,Canada"
   SHIPPED      "Yes"
   INCLUSION_TYPE "Static Link"
   USEABLE_STAND_ALONE "No"
   CONTAINS_CRYPTO "No"
   ECCN         "EAR99"
   ECCN_EVIDENCE "https://sps.sede-coe.pds.nl/CoE-II/Basin Modeling/Release documents/Export Classification Tracking/License and ECCN of Eigen (a C++ linear algebra library).msg"
)

