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

if (UNIX)
	set( FLEXLM_ROOT "FLEXLM-NOTFOUND" CACHE PATH "Path to FlexLM directory" )
	set( FLEXLM_LIBRARY "FLEXLM_LIBRARY-NOTFOUND" CACHE FILEPATH "Path of FlexLM library" )
	set( FLEXLM_INCLUDE_DIR "${FLEXLM_ROOT}/include" CACHE PATH "Path to FlexLM include files" )

        set( FLEXLM_LIBRARIES "${FLEXLM_LIBRARY}")
	set( FLEXLM_INCLUDE_DIRS "${FLEXLM_INCLUDE_DIR}")
endif (UNIX)

# EPTFlexLM is an API to FlexLM for SSS applications. It is supplied by the Global
# Application Deployment (GAD) service. 
add_external_package_info(
      CAPABILITY FlexLMAPI
      NAME    "EPTFlexLM"
      VENDOR  "Shell Internationale Research Maatschappij B.V."
      VERSION "9.2"
      LICENSE_TYPE "Commercial"
      LICENSE_FILE ""
      URL "https://eu001-sp.shell.com/sites/AAAAA4076/RMD/GAD%20service/Forms/AllItems.aspx?RootFolder=%2Fsites%2FAAAAA4076%2FRMD%2FGAD%20service%2FFlexLM%20API&FolderCTID=0x01200024C51524E1BEE648B042C224FA0176A9&View={F3740C25-8079-4661-B216-DB13CDB87BCB}"
      DESCRIPTION "Interface to FlexLM for SSS applications"
      REQUIRED_AT  "Runtime"
      COUNTRY_OF_ORIGIN "The Netherlands"
      SHIPPED      "Yes"
      INCLUSION_TYPE "Static Link"
      USEABLE_STAND_ALONE "No"
      CONTAINS_CRYPTO "No"
      ECCN         "Unknown"
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
      REQUIRED_AT  "Runtime"
      COUNTRY_OF_ORIGIN "USA"
      SHIPPED      "Yes"
      INCLUSION_TYPE "Static Link"
      USEABLE_STAND_ALONE "No"
      CONTAINS_CRYPTO "Yes"
      ECCN         "5D992"
      ECCN_EVIDENCE "https://sps.sede-coe.pds.nl/CoE-II/Basin Modeling/Release documents/Export Classification Tracking/2013.10-ECCN_Discussion_with_George_Jolly.msg"
)

