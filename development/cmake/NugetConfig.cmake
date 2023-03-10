#########################################################################
#                                                                       #
# Copyright (C) 2012-2017 Shell International Exploration & Production. #
# All rights reserved.                                                  #
#                                                                       #
# Developed under license for Shell by PDS BV.                          #
#                                                                       #
# Confidential and proprietary source code of Shell.                    #
# Do not distribute without written permission from Shell.              #
#                                                                       #
#########################################################################

include(cmake/AddPackage.cmake)

set(NUGET_EXECUTABLE "NUGET_EXECUTABLE-NOTFOUND" CACHE PATH "Path to nuget executable") 
set(NUGET_VERSION  "Unknown")

if (UNIX)
   set(VSTS_FEED_NAME Cauldron)
   find_program(NUGET_EXECUTABLE NAMES nuget)
else (UNIX) # Windows
   set(VSTS_FEED_NAME BPA2)
   find_program(NUGET_EXECUTABLE NAMES nuget.exe HINTS "c:/Apps/3rdparty/nuget")
endif (UNIX)

message( STATUS "Found nuget: ${NUGET_EXECUTABLE}")


set(NUGET_VSTS_FEED_URL "https://sede-pt-ssw.pkgs.visualstudio.com/_packaging/${VSTS_FEED_NAME}/nuget/v3/index.json")

if (NUGET_EXECUTABLE)
   set(NUGET_FOUND "Yes") 
else (NUGET_EXECUTABLE)
   message(WARNING "WARNING!!!: Can not find Nuget executable. Nuget package wil not be built.")
endif (NUGET_EXECUTABLE)

if (NUGET_FOUND) 
   execute_process( COMMAND "${NUGET_EXECUTABLE}" OUTPUT_VARIABLE NugetOutput)   
   STRING(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+" NUGET_VERSION ${NugetOutput})
   message(STATUS "Found Nuget: ${NUGET_EXECUTABLE} (found version ${NUGET_VERSION})")

   get_filename_component(NUGET_DIR ${NUGET_EXECUTABLE} DIRECTORY)

   add_external_package_info( 
         CAPABILITY Nuget
         NAME     "Nuget"
         VENDOR   "Microsoft"
         VERSION  "${NUGET_VERSION}"
         LICENSE_TYPE "Apache License, Version 2.0"
         LICENSE_FILE "${NUGET_DIR}/LICENSE"
         URL      "https://github.com/nuget"
         DESCRIPTION "NuGet is a free and open-source package manager designed for the Microsoft development platform."
         REQUIRED_AT  "Build"
         COUNTRY_OF_ORIGIN "USA"
         SHIPPED      "No"
         INCLUSION_TYPE "NA"
         USEABLE_STAND_ALONE "No"
         CONTAINS_CRYPTO "No"
         ECCN         "Unknown"
   )
endif ()
