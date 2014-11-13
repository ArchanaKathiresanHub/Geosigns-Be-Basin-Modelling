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

include( cmake/AddPackage.cmake)


if (UNIX)
   # Convert the compiler name that CMake has to a toolset name that the Boost
   # build system can understand
   if ( CMAKE_CXX_COMPILER_ID STREQUAL GNU )
     set(toolset "gcc")
   else()
     string(TOLOWER "${CMAKE_CXX_COMPILER_ID}" toolset)
   endif()

   # Instruct boost to use our compiler
   add_environment_variable( BOOST_BUILD_PATH "${PROJECT_BINARY_DIR}" )
   file( WRITE "${PROJECT_BINARY_DIR}/user-config.jam" 
         "using ${toolset} : ${CMAKE_CXX_COMPILER_VERSION} : ${CMAKE_CXX_COMPILER} ;\n"
   )

   # Add Boost as an external project
   add_external_project_to_repository(
         NAME Boost
         VERSION 1.56.0
         ARCHIVE "${THIRD_PARTY_DIR}/sources/boost_1_56_0.tar.gz"
         ARCHIVE_MD5 "8c54705c424513fa2be0042696a3a162"
         PATCH_COMMAND 
            "./bootstrap.sh" 
            "--with-libraries=filesystem,system,thread,atomic"
            "--prefix={ROOT}" 
         CONFIGURE_COMMAND "./b2" "install"
         BUILD_COMMAND   "${CMAKE_COMMAND}" "-E" "echo" "Boost has been built."
         INSTALL_COMMAND  "${CMAKE_COMMAND}" "-E" "echo" "Boost has already been installed."
         CONFIGURE_OPTIONS 
           COMPILER "{CurrentCompiler}" "toolset=${toolset}"
           MPI      "{CurrentMPI}"
           SPEED    "Release" "variant=release"
           SPEED    "Debug"   "variant=release"
           SPEED    "DebugAll" "variant=debug"
           SPEED    "MemCheck" "variant=debug"
           OS       "{CurrentPlatform}"
           LINK     "Dynamic" "link=shared" 
           LINK     "Static"  "link=static" 

         YIELD_LIBRARIES  "boost_filesystem" "boost_system" "boost_thread" "boost_atomic" "boost_date_time" "boost_chrono"
   )

   # Use boost in our project
   set(BOOST_ROOT "${Boost_ROOT}")
   set(Boost_FOUND TRUE)
   set(Boost_INCLUDE_DIRS "${BOOST_ROOT}/include")
   set(Boost_LIBRARIES "boost_filesystem" "boost_system"  "boost_thread" "boost_atomic" "boost_date_time" "boost_chrono")

   add_external_package_info(
      CAPABILITY BoostLib
      NAME    "Boost"
      VENDOR  "Boost"
      VERSION "1.56.0"
      LICENSE_TYPE "Boost v1"
      LICENSE_FILE "${THIRD_PARTY_DIR}/licenses/Boost-1.56.0.txt"
      URL "http://boost.org"
      DESCRIPTION "Free peer-reviewed portable C++ source libraries"
      REQUIRED_AT  "Runtime"
      COUNTRY_OF_ORIGIN ""
      SHIPPED      "Yes"
      INCLUSION_TYPE "Static Link"
      USEABLE_STAND_ALONE "No"
      CONTAINS_CRYPTO "No"
      ECCN         "EAR99"
      ECCN_EVIDENCE "https://sps.sede-coe.pds.nl/CoE-II/Basin Modeling/Release documents/Export Classification Tracking/2013.10-ECCN_Discussion_with_George_Jolly.msg"
   )   
   
elseif(WIN32)

    set(BOOST_ROOT "Boost-NOTFOUND" CACHE PATH "Location of the Boost C++ libraries")
    if ( MSVC10 )
        set(BOOST_LIB_POSTFIX "-msvc-10.0")
    elseif(MSVC11)
        set(BOOST_LIB_POSTFIX "-msvc-11.0")
    elseif(MSVC12)
        set(BOOST_LIB_POSTFIX "-msvc-12.0")
    else()
        set(BOOST_LIB_POSTFIX "")
    endif()

    math(EXPR _64 "${CMAKE_SIZEOF_VOID_P} * 8")
    set(BOOST_LIBRARYDIR "${BOOST_ROOT}/lib${_64}${BOOST_LIB_POSTFIX}")
    
    if (NOT BUILD_SHARED_LIBS)
        set(Boost_USE_STATIC_LIBS        ON) # only find static libs
    endif()

    find_package( Boost 1.56.0 REQUIRED COMPONENTS filesystem system thread atomic date_time chrono )

    add_external_package_info(
      CAPABILITY BoostLib
      NAME    "Boost"
      VENDOR  "Boost"
      VERSION "1.56.0"
      LICENSE_TYPE "Boost v1"
      LICENSE_FILE "${THIRD_PARTY_DIR}/licenses/Boost-1.56.0.txt"
      URL "http://boost.org"
      DESCRIPTION "Free peer-reviewed portable C++ source libraries"
      REQUIRED_AT  "Runtime"
      COUNTRY_OF_ORIGIN ""
      SHIPPED      "Yes"
      INCLUSION_TYPE "Static Link"
      USEABLE_STAND_ALONE "No"
      CONTAINS_CRYPTO "No"
      ECCN         "EAR99"
      ECCN_EVIDENCE "https://sps.sede-coe.pds.nl/CoE-II/Basin Modeling/Release documents/Export Classification Tracking/2013.10-ECCN_Discussion_with_George_Jolly.msg"
   )
else()
    message(ERROR "OS not supported")
endif()


include_directories(${Boost_INCLUDE_DIRS})




