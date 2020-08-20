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
           SOURCES test/LegacyBasaltThicknessCalculatorTest.cpp src/LegacyBasaltThicknessCalculator.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../../libraries/DataModel/test/MockValidator.cpp
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
           LIBRARIES utilities
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src
           FOLDER "${BASE_FOLDER}/${TARGET_NAME}"
)
add_gtest( NAME ${TARGET_NAME}::SandstoneToMultipointPermeability
           SOURCES test/SandstoneToMultipointPermeabilityTest.cpp src/SandstoneToMultipointPermeability.cpp
           LIBRARIES utilities
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
		   LIBRARIES cmbapi
           FOLDER "${BASE_FOLDER}/${TARGET_NAME}"
)

#
# 4. Reservoir upgrade testing
#
add_gtest( NAME ${TARGET_NAME}::ReservoirConverter
           SOURCES test/ReservoirConverter.cpp src/ReservoirConverter.cpp
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src
                        ${CMAKE_CURRENT_SOURCE_DIR}/../../libraries/cmbAPI/src
		   LIBRARIES cmbapi
           FOLDER "${BASE_FOLDER}/${TARGET_NAME}"
)

# 5. Biodegradation factor upgarde testing
#
add_gtest( NAME ${TARGET_NAME}::BiodegradeConverter
           SOURCES test/BiodegradeConverterTest.cpp src/BiodegradeConverter.cpp
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src
                        ${CMAKE_CURRENT_SOURCE_DIR}/../../libraries/cmbAPI/src
           LIBRARIES cmbapi 
           FOLDER "${BASE_FOLDER}/${TARGET_NAME}"
)

#
# 6. Porosity upgrade testing
#
add_gtest( NAME ${TARGET_NAME}::PorosityModelConverter
           SOURCES test/PorosityModelConverterTest.cpp src/PorosityModelConverter.cpp
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src
                        ${CMAKE_CURRENT_SOURCE_DIR}/../../libraries/cmbAPI/src
		   LIBRARIES cmbapi
           FOLDER "${BASE_FOLDER}/${TARGET_NAME}"
)

#
# 7. Basic crust thinning model upgrade testing
#
add_gtest( NAME ${TARGET_NAME}::BasicCrustThinningModelConverter
           SOURCES test/BasicCrustThinningConverter.cpp src/BasicCrustThinningModelConverter.cpp
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src
                        ${CMAKE_CURRENT_SOURCE_DIR}/../../libraries/cmbAPI/src
		   LIBRARIES cmbapi
           FOLDER "${BASE_FOLDER}/${TARGET_NAME}"
)

#
# 8. Alc property model upgrade testing
#
add_gtest( NAME ${TARGET_NAME}::AlcModelConverter
           SOURCES test/AlcConverter.cpp src/AlcModelConverter.cpp
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src
                        ${CMAKE_CURRENT_SOURCE_DIR}/../../libraries/cmbAPI/src
           LIBRARIES cmbapi
           FOLDER "${BASE_FOLDER}/${TARGET_NAME}"
)

#
# 9. Ctc model upgrade testing
#
add_gtest( NAME ${TARGET_NAME}::CtcModelConverter
           SOURCES test/CtcConverter.cpp src/CtcModelConverter.cpp
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src
                        ${CMAKE_CURRENT_SOURCE_DIR}/../../libraries/cmbAPI/src
           LIBRARIES cmbapi
           FOLDER "${BASE_FOLDER}/${TARGET_NAME}"
)

# 10. ProjectIoTbl upgrade testing
#
add_gtest( NAME ${TARGET_NAME}::ProjectIoModelConverter
           SOURCES test/ProjectInputConverterTest.cpp src/ProjectIoModelConverter.cpp
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src
                        ${CMAKE_CURRENT_SOURCE_DIR}/../../libraries/cmbAPI/src
           LIBRARIES cmbapi
		   FOLDER "${BASE_FOLDER}/${TARGET_NAME}"
)
#
# 11. top Boundary model upgrade testing
#
add_gtest( NAME ${TARGET_NAME}::TopBoundaryModelConverter
           SOURCES test/TopBoundaryConverterTest.cpp src/TopBoundaryUpgradeManager.cpp
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src
                        ${CMAKE_CURRENT_SOURCE_DIR}/../../libraries/cmbAPI/src
           LIBRARIES cmbapi DataAccess
           FOLDER "${BASE_FOLDER}/${TARGET_NAME}"
)


#
# 12. RunOptions upgrade testing
#
add_gtest( NAME ${TARGET_NAME}::RunOptionsModelConverter
           SOURCES test/RunOptionsConverterTest.cpp src/RunOptionsModelConverter.cpp
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src
                        ${CMAKE_CURRENT_SOURCE_DIR}/../../libraries/cmbAPI/src             
		   LIBRARIES cmbapi
           FOLDER "${BASE_FOLDER}/${TARGET_NAME}"
)

#
# 13. Lithology upgrade testing
#
add_gtest( NAME ${TARGET_NAME}::LithologyConverter
           SOURCES test/LithologyConverterTest.cpp src/LithologyConverter.cpp
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src
                        ${CMAKE_CURRENT_SOURCE_DIR}/../../libraries/cmbAPI/src             
		   LIBRARIES cmbapi
           FOLDER "${BASE_FOLDER}/${TARGET_NAME}"
)

#
# 14. Sgs upgrade testing
#
add_gtest( NAME ${TARGET_NAME}::SgsConverter
           SOURCES test/SgsConverter.cpp src/SgsConverter.cpp
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src
                        ${CMAKE_CURRENT_SOURCE_DIR}/../../libraries/cmbAPI/src             
		   LIBRARIES cmbapi
           FOLDER "${BASE_FOLDER}/${TARGET_NAME}"
)

#
# 15. Source Rock upgrade testing
#
add_gtest( NAME ${TARGET_NAME}::SourceRockConverter
           SOURCES test/SourceRockConverter.cpp src/SourceRockConverter.cpp
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src
                        ${CMAKE_CURRENT_SOURCE_DIR}/../../libraries/cmbAPI/src             
		   LIBRARIES cmbapi
           FOLDER "${BASE_FOLDER}/${TARGET_NAME}"
)

#
# 16. Stratigraphy upgrade testing
#
add_gtest( NAME ${TARGET_NAME}::StratigraphyConverter
           SOURCES test/StratigraphyModelConverterTest.cpp src/StratigraphyModelConverter.cpp
           INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src
                        ${CMAKE_CURRENT_SOURCE_DIR}/../../libraries/cmbAPI/src             
		   LIBRARIES cmbapi
           FOLDER "${BASE_FOLDER}/${TARGET_NAME}"
)
