#########################################################################
#                                                                       #
# Copyright (C) 2012-2018 Shell International Exploration & Production. #
# All rights reserved.                                                  #
#                                                                       #
# Developed under license for Shell by PDS BV.                          #
#                                                                       #
# Confidential and proprietary source code of Shell.                    #
# Do not distribute without written permission from Shell.              #
#                                                                       #
#########################################################################

################################################################
######################### SET TARGETS ##########################
################################################################

FILE( GLOB all_headers src/*.h )
source_group(include FILES ${all_headers})

FILE( GLOB all_srcs src/*.C src/*.cpp)
source_group(source FILES ${all_srcs})

set( TARGET_NAME "prograde" )
add_executable( ${TARGET_NAME}
   ${all_srcs}
   ${all_headers}
) 
set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER "${BASE_FOLDER}/${TARGET_NAME}" )

################################################################
################### SET DEPENDENCIES/LINKS #####################
################################################################

bm_include_libraries(
   cmbAPI
   CBMGenerics
   FileSystem
   TableIO
   DataModel
   DataAccess
   SerialDataAccess
   Serial_Hdf5
   utilities
   ${HDF5_LIBRARIES}
)

include_directories( SYSTEM
   ${SUMPP_INCLUDE_DIR}
   ${HDF5_INCLUDE_DIRS}
)

target_link_libraries( ${TARGET_NAME}
   cmbapi
   CBMGenerics
   DataModel
   DataAccess
   SerialDataAccess
   Serial_Hdf5
   TableIO
   utilities
   ${HDF5_LIBRARIES}
   ${Boost_LIBRARIES}
)

install( TARGETS ${TARGET_NAME}
         DESTINATION bin
)

################################################################
######################## UNIT TESTS ############################
################################################################


#
# 1. ALC upgrade testing
#
add_gtest( NAME ${TARGET_NAME}::LegacyBasaltThicknessCalculator
           SOURCES test/LegacyBasaltThicknessCalculatorTest.cpp src/LegacyBasaltThicknessCalculator.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../../libraries/DataModel/test/MockValidator.C
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src
                        ${CMAKE_CURRENT_SOURCE_DIR}/../../libraries/SerialDataAccess/src/Interface
                        ${CMAKE_CURRENT_SOURCE_DIR}/../../libraries/DataModel/test
                        ${CMAKE_CURRENT_SOURCE_DIR}/../../libraries/CBMGenerics/src
                        ${CMAKE_CURRENT_SOURCE_DIR}/../../libraries/utilities/src
           LIBRARIES DataAccess SerialDataAccess DataModel CBMGenerics utilities
           FOLDER "${BASE_FOLDER}/${TARGET_NAME}"
)

#
# 2. Permeability upgrade testing
#
add_gtest( NAME ${TARGET_NAME}::MudstoneToMultipointPermeability
           SOURCES test/MudstoneToMultipointPermeabilityTest.cpp src/MudstoneToMultipointPermeability.cpp
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src
           FOLDER "${BASE_FOLDER}/${TARGET_NAME}"
)
add_gtest( NAME ${TARGET_NAME}::SandstoneToMultipointPermeability
           SOURCES test/SandstoneToMultipointPermeabilityTest.cpp src/SandstoneToMultipointPermeability.cpp
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src
           FOLDER "${BASE_FOLDER}/${TARGET_NAME}"
)


#
# 3. Brine upgrade testing
#
add_gtest( NAME ${TARGET_NAME}::BrineModelConverter
           SOURCES test/BrineModelConverterTest.cpp src/BrineModelConverter.cpp
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src
                        ${CMAKE_CURRENT_SOURCE_DIR}/../../libraries/cmbAPI/src
                        ${CMAKE_CURRENT_SOURCE_DIR}/../../libraries/utilities/src
		   LIBRARIES cmbapi utilities
           FOLDER "${BASE_FOLDER}/${TARGET_NAME}"
)

#
# 4. Biodegradation factor upgarde testing
#
add_gtest( NAME ${TARGET_NAME}::BiodegradeConverter
           SOURCES test/BiodegradeConverterTest.cpp src/BiodegradeConverter.cpp
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src
                        ${CMAKE_CURRENT_SOURCE_DIR}/../../libraries/cmbAPI/src
                        ${CMAKE_CURRENT_SOURCE_DIR}/../../libraries/utilities/src
                   LIBRARIES cmbapi utilities
           FOLDER "${BASE_FOLDER}/${TARGET_NAME}"
)