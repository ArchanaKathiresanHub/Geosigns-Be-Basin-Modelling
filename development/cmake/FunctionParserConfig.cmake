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

set( FP_ROOT "/glb/home/ksaho3/FunctionParser/src" CACHE PATH "Path to Function Parser library" )
set( FP_LIBRARIES "/glb/home/ksaho3/FunctionParser/src/libparser.a" CACHE PATH "Path to Function Parser library" )
set( FP_INCLUDE_DIRS "/glb/home/ksaho3/FunctionParser/src" CACHE PATH "Path to Function Parser library include directory" )

add_external_package_info(
      CAPABILITY FunctionParser
      NAME    "FunctionParser"
      VENDOR  "Juha Nieminen, Joel Yliluoma"
      VERSION "4.0.5"
      LICENSE_TYPE "LGPL v3"
      LICENSE_FILE "/glb/home/ksaho3/FunctionParser/src/lgpl.txt"
      URL "http://warp.povusers.org/FunctionParser/"
      DESCRIPTION "Function Parser for C++"
      REQUIRED_AT  "Runtime"
      COUNTRY_OF_ORIGIN "Finland"
      SHIPPED      "Yes"
      INCLUSION_TYPE "Static Link"
      USEABLE_STAND_ALONE "No"
      CONTAINS_CRYPTO "No"
      ECCN         "Unknown"
)

