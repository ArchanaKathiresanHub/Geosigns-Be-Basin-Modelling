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
# Use Intel MKL as BLAS library
#

set( BLAS_FOUND "ON" CACHE BOOL "Whether a BLAS library is available" )
set( INTEL_MKL_ROOT "${INTEL_CXX_ROOT}/mkl" )
set( BLAS_ROOT "${INTEL_MKL_ROOT}" CACHE PATH "Path to BLAS library" )

set( MKL_LIBRARIES)
   list( APPEND MKL_LIBRARIES  "-Wl,--start-group")
   list( APPEND MKL_LIBRARIES "${INTEL_MKL_ROOT}/lib/intel64/libmkl_intel_lp64.a")
   list( APPEND MKL_LIBRARIES "${INTEL_MKL_ROOT}/lib/intel64/libmkl_sequential.a")
   list( APPEND MKL_LIBRARIES "${INTEL_MKL_ROOT}/lib/intel64/libmkl_core.a")
   list( APPEND MKL_LIBRARIES "-Wl,--end-group" ) 
set ( BLAS_LIBRARIES "${MKL_LIBRARIES}" CACHE STRING "List of libraries that have to be linked to use BLAS" )

set( BLAS95_FOUND "OFF" CACHE BOOL "Whether Fortran 95 interface for BLAS is provided" )

add_external_package_info( 
       CAPABILITY   BLAS
       NAME         "MKL"
       VENDOR       "Intel"
       VERSION      "11.0.4.192"
       LICENSE_TYPE "Commercial"
       LICENSE_FILE "${INTEL_CXX_ROOT}/Documentation/en_US/clicense"
       URL          "http://software.intel.com/en-us/intel-mkl"
       DESCRIPTION  "Intel Math Kernel Library"
       REQUIRED_AT  "Runtime"
       COUNTRY_OF_ORIGIN "USA"
       SHIPPED      "Yes"
       INCLUSION_TYPE "Static Link"
       USEABLE_STAND_ALONE "No"
       CONTAINS_CRYPTO "No"
       ECCN         "5D992"
       ECCN_EVIDENCE "https://sps.sede-coe.pds.nl/CoE-II/Basin Modeling/Release documents/Export Classification Tracking/2013.10-ECCN_Discussion_with_George_Jolly.msg"
)                   

