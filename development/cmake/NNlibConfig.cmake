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

# Include NNlib library. Shell library for SA/UA 

set( NNLIB_ROOT "${THIRD_PARTY_DIR}/NNlib" CACHE PATH "Path to NNlib sources" )
set( NNLIB_INCLUDE_DIR "${NNLIB_ROOT}/include")

set( NNLIB_FOUND TRUE )

add_subdirectory(${THIRD_PARTY_DIR}/NNlib NNlib)

add_external_package_info(
       CAPABILITY NNlib
       NAME     "nn"
       VENDOR   "Pavel Sakov and CSIRO"
       VERSION  "v. 1.86.0"
       LICENSE_TYPE "BSD"
       LICENSE_FILE "${THIRD_PARTY_DIR}/NNlib/LICENSE_NN"
       URL      "https://github.com/sakov/nn-c"
       DESCRIPTION "Performs natural neighbours interpolation"
       REQUIRED_AT  "Runtime"
       COUNTRY_OF_ORIGIN "Australia"
       SHIPPED      "Yes"
       INCLUSION_TYPE "Static Link"
       USEABLE_STAND_ALONE "No"
       CONTAINS_CRYPTO "No"
       ECCN         "EAR99"
       ECCN_EVIDENCE "https://sps.sede-coe.pds.nl/CoE-II/Basin Modeling/Release documents/Export Classification Tracking/Clarification on licensing issues_nn_poly2tri.msg"
)
