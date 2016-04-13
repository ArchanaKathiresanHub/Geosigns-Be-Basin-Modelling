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
      VERSION 3.2.8
      ARCHIVE "${THIRD_PARTY_DIR}/sources/eigen-3.2.8.tar.gz"
      ARCHIVE_MD5 "59df18a22e481c44025428b524d04312"
      CONFIGURE_COMMAND "./configure.sh" "config" "-DCMAKE_INSTALL_PREFIX={ROOT}"
      BUILD_COMMAND   "./configure.sh" "build"         
      INSTALL_COMMAND "./configure.sh" "install"
      CONFIGURE_OPTIONS 
        COMPILER "{CurrentCompiler}" "-DCMAKE_CXX_COMPILER={CXX}" "-DCMAKE_CC_COMPILER={CC}"
        MPI      "{CurrentMPI}"
        SPEED    "Release"      "-DCMAKE_BUILD_TYPE=Release"
        SPEED    "Debug"        "-DCMAKE_BUILD_TYPE=Debug"
        SPEED    "DebugAll"     "-DCMAKE_BUILD_TYPE=Debug"
        SPEED    "MemCheck"     "-DCMAKE_BUILD_TYPE=Debug"
        SPEED    "CodeCoverage" "-DCMAKE_BUILD_TYPE=Release"
        OS       "{CurrentPlatform}"
        LINK     "Dynamic"
        LINK     "Static"
)

set(EIGEN_INCLUDE_DIRS "${Eigen_ROOT}" "${Eigen_ROOT}/include" "${Eigen_ROOT}/include/eigen3")

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


