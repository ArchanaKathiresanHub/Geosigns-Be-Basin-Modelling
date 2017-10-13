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

################################################################
##################### SET COMPILER FLAGS #######################
################################################################

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
  set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -axCORE-AVX2 -qopenmp -simd")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
# Vectorization is not enabled on Visual Studio
#  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()

################################################################
######################### SET TARGETS ##########################
################################################################

FILE( GLOB all_headers src/*.h )
source_group(include FILES ${all_headers})

FILE( GLOB all_srcs src/*.C )
source_group(source FILES ${all_srcs})

set( LIB_NAME "utilities" )
add_library(${LIB_NAME}
   ${all_srcs}
   ${all_headers}
)
set_target_properties( ${LIB_NAME} PROPERTIES FOLDER "${BASE_FOLDER}/${LIB_NAME}" )

################################################################
################### SET DEPENDENCIES/LINKS #####################
################################################################

# under Linux we are building Boost that is why this target exist
if (UNIX)
   add_dependencies(${LIB_NAME} Boost)
endif (UNIX)


target_link_libraries(${LIB_NAME}
   ${Boost_LIBRARIES}
)

install(TARGETS ${LIB_NAME}
   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)

################################################################
######################## UNIT TESTS ############################
################################################################

######################
# AlignedWorkSpaceArray tests
add_gtest( NAME ${LIB_NAME}::AlignedWorkSpaceArrays
   SOURCES test/AlignedWorkSpaceArraysTest.C
   LIBRARIES ${LIB_NAME}
   FOLDER "${BASE_FOLDER}/${LIB_NAME}")

######################
# PiecewiseInterpolator tests
add_gtest( NAME ${LIB_NAME}::PiecewiseInterpolator
   SOURCES test/PiecewiseInterpolatorTest.C
   LIBRARIES ${LIB_NAME}
   FOLDER "${BASE_FOLDER}/${LIB_NAME}")

add_gtest( NAME ${LIB_NAME}::PiecewiseInterpolatorDerivative
   SOURCES test/PiecewiseInterpolatorDerivativeTest.C
   LIBRARIES ${LIB_NAME}
   FOLDER "${BASE_FOLDER}/${LIB_NAME}")

######################
# IBSInterpolator tests
add_gtest( NAME ${LIB_NAME}::IBSinterpolator
   SOURCES test/IBSinterpolator.C
   LIBRARIES ${LIB_NAME}
   FOLDER "${BASE_FOLDER}/${LIB_NAME}")

add_gtest( NAME ${LIB_NAME}::IBSinterpolator2d
   SOURCES test/IBSinterpolator2d.C
   LIBRARIES ${LIB_NAME}
   FOLDER "${BASE_FOLDER}/${LIB_NAME}")

######################
# LogHandler tests
add_gtest( NAME ${LIB_NAME}::LogHandlerCreate
   SOURCES test/LogHandlerCreate.C test/LogHandlerUnitTester.h
   LIBRARIES ${LIB_NAME}
   FOLDER "${BASE_FOLDER}/${LIB_NAME}")

add_gtest( NAME ${LIB_NAME}::LogHandlerDiagnostic
   SOURCES test/LogHandlerDiagnostic.C test/LogHandlerUnitTester.h
   LIBRARIES ${LIB_NAME}
   FOLDER "${BASE_FOLDER}/${LIB_NAME}")

add_gtest( NAME ${LIB_NAME}::LogHandlerDetailed
   SOURCES test/LogHandlerDetailed.C test/LogHandlerUnitTester.h
   LIBRARIES ${LIB_NAME}
   FOLDER "${BASE_FOLDER}/${LIB_NAME}")

add_gtest( NAME ${LIB_NAME}::LogHandlerNormal
   SOURCES test/LogHandlerNormal.C test/LogHandlerUnitTester.h
   LIBRARIES ${LIB_NAME}
   FOLDER "${BASE_FOLDER}/${LIB_NAME}")

add_gtest( NAME ${LIB_NAME}::LogHandlerQuiet
   SOURCES test/LogHandlerQuiet.C test/LogHandlerUnitTester.h
   LIBRARIES ${LIB_NAME}
   FOLDER "${BASE_FOLDER}/${LIB_NAME}")

add_gtest( NAME ${LIB_NAME}::LogHandlerMinimal
   SOURCES test/LogHandlerMinimal.C test/LogHandlerUnitTester.h
   LIBRARIES ${LIB_NAME}
   FOLDER "${BASE_FOLDER}/${LIB_NAME}")

add_gtest( NAME ${LIB_NAME}::LogHandlerStyles
   SOURCES test/LogHandlerStyles.C test/LogHandlerUnitTester.h
   LIBRARIES ${LIB_NAME}
   FOLDER "${BASE_FOLDER}/${LIB_NAME}")

add_gtest( NAME ${LIB_NAME}::LogHandlerTime
   SOURCES test/LogHandlerTime.C test/LogHandlerUnitTester.h
   LIBRARIES ${LIB_NAME}
   FOLDER "${BASE_FOLDER}/${LIB_NAME}")

######################
# StringHandler tests
add_gtest( NAME ${LIB_NAME}::StringHandler
   SOURCES test/StringHandler.C
   LIBRARIES ${LIB_NAME}
   FOLDER "${BASE_FOLDER}/${LIB_NAME}")