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

set(Eigen_ROOT "${THIRD_PARTY_DIR}/Eigen-3.2.8" CACHE PATH "Path to Eigen include files" )
set(EIGEN_INCLUDE_DIRS "${Eigen_ROOT}" "${Eigen_ROOT}/eigen3")

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


