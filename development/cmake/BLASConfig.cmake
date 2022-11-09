#########################################################################
#                                                                       #
# Copyright (C) 2012-2016 Shell International Exploration & Production. #
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

message(STATUS "BLAS vendor is set to ${BLA_VENDOR}" )

set(INTEL_MKL_ROOT "INTEL_MKL_ROOT-NOTFOUND" CACHE PATH "Path to Intel MKL" )
set(INTEL_MKL_VERSION "oneMKL 2021.2" CACHE STRING "Intel MKL version")

if (UNIX)
   if ( BLA_VENDOR STREQUAL "MKL" )
      set( BLAS_INCLUDE_DIRS "${BLAS_ROOT}/include" )
      set( BLAS_ROOT "${INTEL_MKL_ROOT}" CACHE PATH "Path to BLAS library" )
      if ( INTEL_MKL_ROOT )
         set( MKL_LIBRARIES
            "-Wl,--start-group"
            "-Wl,${INTEL_MKL_ROOT}/lib/intel64/libmkl_intel_lp64.a"
            "-Wl,${INTEL_MKL_ROOT}/lib/intel64/libmkl_core.a"
            "-Wl,${INTEL_MKL_ROOT}/lib/intel64/libmkl_sequential.a"
            "-Wl,${INTEL_MKL_ROOT}/lib/intel64/libmkl_blacs_intelmpi_lp64.a"
            "-Wl,--end-group" 
         )
         set( BLAS_FOUND ON )
         #set( BLAS_ROOT "${INTEL_MKL_ROOT}" CACHE PATH "Path to BLAS library" ) # To be used if BLAS is not found from path
      else()
         set( MKL_LIBRARIES "BLAS_LIBRARIES-NOTFOUND")
         set( BLAS_FOUND OFF )
      endif()

   elseif( BLA_VENDOR STREQUAL "ATLAS" )
      if ( EXISTS "/usr/lib64/atlas-sse3" )
         set( BLAS_ROOT "/usr/lib64/atlas-sse3" CACHE PATH "Path to BLAS library" )
      elseif ( EXISTS "/usr/lib64/atals" )
         set( BLAS_ROOT "/usr/lib64/atlas" CACHE PATH "Path to BLAS library" )
      else ()
         message( FATAL_ERROR "Can't find ATLAS library" )
      endif()
      set( MKL_LIBRARIES "-L${BLAS_ROOT} -llapack -latlas" )
      set( BLAS_FOUND ON )

   elseif( BLA_VENDOR STREQUAL "OPENBLAS" )
      set( BLAS_ROOT "/usr/lib64" CACHE PATH "Path to BLAS library" )
      set( MKL_LIBRARIES "-L${BLAS_ROOT} -llapack -lblas" )
      set( BLAS_FOUND ON )

   else()
      set( MKL_LIBRARIES "BLAS_LIBRARIES-NOTFOUND")
      set( BLAS_FOUND OFF )
   endif()

elseif(WIN32)
   set( BLAS_INCLUDE_DIRS )
   set( MKL_LIBRARIES
          "${INTEL_MKL_ROOT}/mkl_intel_lp64.lib"
          "${INTEL_MKL_ROOT}/mkl_intel_thread.lib"
          "${INTEL_MKL_ROOT}/mkl_core.lib"
          "${INTEL_MKL_ROOT}/libiomp5md.lib"
      )
   set( BLAS_FOUND ON )
endif()

set ( BLAS_LIBRARIES "${MKL_LIBRARIES}" CACHE STRING "List of libraries that have to be linked to use BLAS" )

set( BLAS95_FOUND "OFF" CACHE BOOL "Whether Fortran 95 interface for BLAS is provided" )

add_external_package_info( 
       CAPABILITY   BLAS
       NAME         "MKL"
       VENDOR       "Intel"
       VERSION      "${INTEL_MKL_VERSION}"
       LICENSE_TYPE "Commercial"
       LICENSE_FILE "${INTEL_MKL_ROOT}/licensing/license.txt"
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

