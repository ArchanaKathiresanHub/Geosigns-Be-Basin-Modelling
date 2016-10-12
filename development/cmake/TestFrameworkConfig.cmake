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

set( BM_UNIT_TEST_OUTPUT_DIR "${PROJECT_BINARY_DIR}" CACHE PATH "Directory where XML results of the Unit Tests go")

# Include Google Testing Framework (which is itself a CMake project)
# and Google Mock (Google Mock contains Google Test)

option( gtest_force_shared_crt
   "Use shared (DLL) run-time lib even when Google Test is built as static lib."
   ON
)

# MS Visual Studio 2012 doesnt support variadic templates
if (MSVC11)
  add_definitions(-D_VARIADIC_MAX=10)
endif()
    
add_subdirectory(${THIRD_PARTY_DIR}/gmock-1.6.0 gmock EXCLUDE_FROM_ALL)

# Mark internal Google test variables as advanced
mark_as_advanced( 
      gmock_build_tests
      gtest_build_samples
      gtest_build_tests
      gtest_disable_pthreads
      gtest_force_shared_crt
)

# General pacakage information
add_external_package_info( 
       CAPABILITY TestFramework
       NAME     "Google Test"
       VENDOR   "Google, Inc."
       VERSION  "1.6.0"
       LICENSE_TYPE "BSD 3-Clause"
       LICENSE_FILE "${THIRD_PARTY_DIR}/gmock-1.6.0/gtest/COPYING"
       URL      "https://code.google.com/p/googletest"
       DESCRIPTION "Google's Unit Testing framework"
       REQUIRED_AT  "Test"
       COUNTRY_OF_ORIGIN "USA"
       SHIPPED      "No"
       INCLUSION_TYPE "NA"
       USEABLE_STAND_ALONE "No"
       CONTAINS_CRYPTO "No"
       ECCN         "Unknown"
)                   

add_external_package_info( 
       CAPABILITY MockFramework
       NAME     "Google Mock"
       VENDOR   "Google, Inc."
       VERSION  "1.6.0"
       LICENSE_TYPE "BSD 3-Clause"
       LICENSE_FILE "${THIRD_PARTY_DIR}/gmock-1.6.0/COPYING"
       URL      "https://code.google.com/p/googlemock"
       DESCRIPTION "Google's Mocking framework"
       REQUIRED_AT  "Test"
       COUNTRY_OF_ORIGIN "USA"
       SHIPPED      "No"
       INCLUSION_TYPE "NA"
       USEABLE_STAND_ALONE "No"
       CONTAINS_CRYPTO "No"
       ECCN         "Unknown"
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
   set(mpiSize)     # The number of MPI processes
   set(include_dirs)# Additional include directories
   set(environment_vars)#List of variables in format VAR=VAL

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
      elseif(param STREQUAL MPI_SIZE)
         set(parameterName mpiSize)
      elseif(param STREQUAL INCLUDE_DIRS)
         set(parameterName include_dirs)
      elseif(param STREQUAL ENV_VARS)
         set(parameterName environment_vars)        
      else()
         list(APPEND ${parameterName} ${param})
      endif()
   endforeach()

   ### Add the test
   # Make a suitable executable / target name by replacing funny charateres
   # with an underscore
   string( REGEX REPLACE "[^A-Za-z0-9]" "_" execName "Test${testName}")
   
   # Windows has a 260 character limitation of path names, so want to keep the executable name length limited
   if (WIN32)
     set(maxExeNameLength 40)
	  string(LENGTH "${execName}" exeNameLength)
     if (exeNameLength GREATER maxExeNameLength)
	    string(SUBSTRING "${execName}" 0 "${maxExeNameLength}" croppedExeName)

       if( NOT DEFINED RANDOM_SEED_VAL_TEST )
         set(RANDOM_SEED_VAL_TEST 1970 CACHE INTERNAL "Seed value for the test random name generation" )
   		 string(RANDOM LENGTH 8 RANDOM_SEED ${RANDOM_SEED_VAL_TEST} randomSuffix)
       else()
   		 string(RANDOM LENGTH 8 randomSuffix)
       endif()
		 set( execName "${croppedExeName}_${randomSuffix}")
	  endif()
   endif()

   # Add Google Mock (which includes Google Test also) to the list of
   # libraries
   list(APPEND libraries "gmock_main")

   # Add the test executable with its sources
   add_executable( ${execName} ${sources})

   # Link with the necessary libraries
   target_link_libraries( ${execName} ${libraries})
   
   # MS Visual Studio 2012 doesnt support variadic templates
   if (MSVC11)
      add_definitions(-D_VARIADIC_MAX=10)
   endif()
   
   # Add the Google Mock and Google Test include directories
   get_property(incdirs TARGET ${execName} PROPERTY INCLUDE_DIRECTORIES)
   target_include_directories( ${execName} SYSTEM PRIVATE "${TESTFRAMEWORK_INCLUDE_DIRS}" )
   target_include_directories( ${execName} PRIVATE "${incdirs};${include_dirs}" )
   set_target_properties( ${execName} 
      PROPERTIES 
                 COMPILE_FLAGS "${compileflags}"
                 LINK_FLAGS "${linkflags}"   )

   # Add the test to the CTest test  collection.
   if (mpiSize)
     set( mpiCommand "${MPIRUN}" )

     math( EXPR maxProcValue "${mpiSize} - 1")
     foreach( rank RANGE ${maxProcValue} )
        if ( NOT rank EQUAL 0 )
           set( mpiCommand ${mpiCommand} ":" )
        endif()

        set( mpiCommand ${mpiCommand} -n 1 $<TARGET_FILE:${execName}> --gtest_output=xml:${BM_UNIT_TEST_OUTPUT_DIR}/${execName}-${rank}-junit.xml )
     endforeach()

     add_test(NAME "UNIT-TEST-${testName}" COMMAND ${mpiCommand} )
   else ()
     add_test("UNIT-TEST-${testName}" ${execName}  "--gtest_output=xml:${BM_UNIT_TEST_OUTPUT_DIR}/${execName}-junit.xml")
   endif()
   
   if (environment_vars)
      set_tests_properties(UNIT-TEST-${testName} PROPERTIES  ENVIRONMENT "${environment_vars}")
   endif(environment_vars)   
   
endmacro(add_gtest)

