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

set( LZ_ROOT "${THIRD_PARTY_DIR}/lz4" CACHE PATH "Path to LZ4 sources" )
set( LZ_INCLUDE_DIR "${LZ_ROOT}/include")

set( LZ_FOUND TRUE )

add_subdirectory(${THIRD_PARTY_DIR}/lz4 lz4)

add_external_package_info(
       CAPABILITY LZ
       NAME     "LZ4"
       VENDOR   "Yann Collet"
       VERSION  "1.7.1"
       LICENSE_TYPE "BSD"
       LICENSE_FILE "${THIRD_PARTY_DIR}/licenses/lz4.txt"
       URL      "http://lz4.github.io/lz4/"
       DESCRIPTION "LZ4 is a lossless data compression algorithm"
       REQUIRED_AT  "Runtime"
       COUNTRY_OF_ORIGIN "EU"
       SHIPPED      "Yes"
       INCLUSION_TYPE "Static Link"
       USEABLE_STAND_ALONE "No"
       CONTAINS_CRYPTO "No"
       ECCN         "Unknown"
)
