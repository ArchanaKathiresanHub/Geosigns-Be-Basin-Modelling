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

set( FLEXLM_ROOT "/apps/3rdparty/EPTFlexLm/v9.2" CACHE PATH "Path to FlexLM directory" )
set( FLEXLM_LIBRARIES "/apps/3rdparty/EPTFlexLm/v9.2/LinuxRHEL_i686_30WS/lib64/EPTFlexLm.o" CACHE PATH "Path of FlexLM libraries" )
set( FLEXLM_INCLUDE_DIRS "/apps/3rdparty/EPTFlexLm/v9.2/include" CACHE PATH "Path of FlexLM include files" )

add_external_package_info(
      CAPABILITY FlexLMAPI
      NAME    "EPTFlexLM"
      VENDOR  "Shell Internationale Research Maatschappij B.V."
      VERSION "9.2"
      LICENSE_TYPE "Commercial"
      LICENSE_FILE ""
      URL ""
      DESCRIPTION "Interface between a SEPTAR application and the license management tool FLEXlm"
      DEPENDENCY "Runtime"
)

# Probably, this object file was built with help from /apps/3rdparty/flexlm/v9.2.3/root/machind
add_external_package_info(
      CAPABILITY FlexLM
      NAME    "FlexLM"
      VENDOR  "Flexera Software"
      VERSION "9.2.3"
      LICENSE_TYPE "Commercial"
      LICENSE_FILE ""
      URL "http://www.flexerasoftware.com/products/entitlement-management/flexnet-producer-suite-software-vendors/flexnet-licensing/"
      DESCRIPTION ""
      DEPENDENCY "Runtime"
)

