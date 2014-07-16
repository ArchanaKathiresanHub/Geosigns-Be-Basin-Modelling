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

set(EIGEN_VERSION "3.2" CACHE STRING "Eigen Version")
set(EIGEN_ROOT "${HPC_HOME}/eigen/${EIGEN_VERSION}/LinuxRHEL64" CACHE PATH "Path to the Eigen C++ Linear algebra library")

if(UNIX)
	set(EIGEN_INCLUDE_DIRS "${EIGEN_ROOT}/include")
else(UNIX)
	set(EIGEN_INCLUDE_DIRS "${THIRD_PARTY_DIR}/Eigen-3.2.1")
endif(UNIX)

set(EIGEN_FOUND TRUE)

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

