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

# Include Google Testing Framework (which is itself a CMake project)
# and Google Mock (Google Mock contains Google Test)


option( gtest_force_shared_crt
   "Use shared (DLL) run-time lib even when Google Test is built as static lib."
   ON
)
     
add_subdirectory(${PROJECT_SOURCE_DIR}/../3rdparty/gmock-1.6.0 gmock EXCLUDE_FROM_ALL)

add_external_package_info( 
       CAPABILITY TestFramework
       NAME     "Google Test"
       VENDOR   "Google, Inc."
       VERSION  "1.6.0"
       LICENSE_TYPE "BSD 3-Clause"
       LICENSE_FILE "${PROJECT_SOURCE_DIR}/../3rdparty/gmock-1.6.0/gtest/COPYING"
       URL      "https://code.google.com/p/googletest"
       DESCRIPTION "Google's Unit Testing framework"
       DEPENDENCY "Test"
)                   

add_external_package_info( 
       CAPABILITY MockFramework
       NAME     "Google Mock"
       VENDOR   "Google, Inc."
       VERSION  "1.6.0"
       LICENSE_TYPE "BSD 3-Clause"
       LICENSE_FILE "${PROJECT_SOURCE_DIR}/../3rdparty/gmock-1.6.0/COPYING"
       URL      "https://code.google.com/p/googlemock"
       DESCRIPTION "Google's Mocking framework"
       DEPENDENCY "Test"
)                   
                
set(TESTFRAMEWORK_FOUND TRUE)
set(TESTFRAMEWORK_INCLUDE_DIRS "${gtest_SOURCE_DIR}/include;${gmock_SOURCE_DIR}/include")

macro(add_gtest )

   ### Parse parameters
   set(testName)    # The name of the test
   set(sources)     # The source files
   set(libraries)   # The libraries that should be linked with it
   set(compileflags)# The set of compilator flags
   set(linkflags)   # The set of linker flags

   set(parameterName)
   foreach(param ${ARGN})
      if (param STREQUAL NAME)
         set(parameterName testName)
      elseif(param STREQUAL SOURCES)
         set(parameterName sources)
      elseif(param STREQUAL LIBRARIES)
         set(parameterName libraries)
      elseif(param STREQUAL COMPILE_FLAGS)
         set(parameterName compileflags)
      elseif(param STREQUAL LINK_FLAGS)
         set(parameterName linkflags)
      else()
         list(APPEND ${parameterName} ${param})
      endif()
   endforeach()

   ### Add the test
   # Make a suitable executable / target name by replacing funny charateres
   # with an underscore
   string( REGEX REPLACE "[^A-Za-z0-9]" "_" execName "Test${testName}")

   # Add Google Mock (which includes Google Test also) to the list of
   # libraries
   list(APPEND libraries "gmock_main")

   # Add the test executable with its sources
   add_executable( ${execName} ${sources})

   # Link with the necessary libraries
   target_link_libraries( ${execName} ${libraries})
   
   # Add the Google Mock and Google Test include directories
   get_property(incdirs TARGET ${execName} PROPERTY INCLUDE_DIRECTORIES)
   set_target_properties( ${execName} 
      PROPERTIES 
                 INCLUDE_DIRECTORIES "${TESTFRAMEWORK_INCLUDE_DIRS};${incdirs}"
                 COMPILE_FLAGS "${compileflags}"
                 LINK_FLAGS "${linkflags}"   )

   # Add the test to the CTest test  collection.
   add_test(${testName} ${execName}  "--gtest_output=xml:${PROJECT_BINARY_DIR}/${execName}-junit.xml")
endmacro(add_gtest)

