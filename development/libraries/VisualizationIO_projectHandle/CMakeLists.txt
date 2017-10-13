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

FILE( GLOB all_headers src/*.h src/*.hpp)
source_group("Header Files" FILES ${all_headers})

FILE( GLOB all_srcs src/*.cpp)
source_group("Source Files" FILES ${all_srcs})

set( LIB_NAME "VisualizationIO_projectHandle" )
add_library(${LIB_NAME} ${all_srcs} ${all_headers})
set_target_properties( ${LIB_NAME} PROPERTIES FOLDER "${BASE_FOLDER}/${LIB_NAME}" )

include_directories( SYSTEM
   ${HDF5_INCLUDE_DIRS}
   ${PUGIXML_INCLUDE_DIR}
   ${LZ_INCLUDE_DIR}
)

bm_include_libraries(
   DataModel
   DataAccess
   CBMGenerics
   utilities
   SerialDataAccess
   FileSystem 
   DataAccess
   VisualizationIO
   ${LIB_NAME}
   TableIO
)

target_link_libraries(${LIB_NAME}
   DataModel
   DataAccess
   utilities
   SerialDataAccess
   FileSystem 
   DataAccess
   TableIO
   VisualizationIO
   PugiXMLlib
   LZlib
   ${HDF5_LIBRARIES}
   ${Boost_LIBRARIES}
)

# copy some unit test data
configure_file(test/data/ARD_simple-test.project3d ${CMAKE_CURRENT_BINARY_DIR}/data/ARD_simple-test.project3d COPYONLY)

configure_file(test/data/Inputs.HDF ${CMAKE_CURRENT_BINARY_DIR}/data/Inputs.HDF COPYONLY)

configure_file(test/data/ARD_simple-test_MassBalance ${CMAKE_CURRENT_BINARY_DIR}/data/ARD_simple-test_MassBalance COPYONLY)

configure_file(test/data/ARD_simple-test_CauldronOutputDir/Genex5_Results.HDF ${CMAKE_CURRENT_BINARY_DIR}/data/ARD_simple-test_CauldronOutputDir/Genex5_Results.HDF COPYONLY)

configure_file(test/data/ARD_simple-test_CauldronOutputDir/History_ARD_simple-test_genex_sr_5500_4500.dat ${CMAKE_CURRENT_BINARY_DIR}/data/ARD_simple-test_CauldronOutputDir/History_ARD_simple-test_genex_sr_5500_4500.dat COPYONLY)

configure_file(test/data/ARD_simple-test_CauldronOutputDir/HistoryGenex.txt ${CMAKE_CURRENT_BINARY_DIR}/data/ARD_simple-test_CauldronOutputDir/HistoryGenex.txt COPYONLY)

configure_file(test/data/ARD_simple-test_CauldronOutputDir/MyWell.BHF ${CMAKE_CURRENT_BINARY_DIR}/data/ARD_simple-test_CauldronOutputDir/MyWell.BHF COPYONLY)

configure_file(test/data/ARD_simple-test_CauldronOutputDir/HighResDecompaction_Results.HDF ${CMAKE_CURRENT_BINARY_DIR}/data/ARD_simple-test_CauldronOutputDir/HighResDecompaction_Results.HDF COPYONLY)

configure_file(test/data/ARD_simple-test_CauldronOutputDir/HighResMigration_Results.HDF ${CMAKE_CURRENT_BINARY_DIR}/data/ARD_simple-test_CauldronOutputDir/HighResMigration_Results.HDF COPYONLY)

configure_file(test/data/ARD_simple-test_CauldronOutputDir/PressureAndTemperature_Results.HDF ${CMAKE_CURRENT_BINARY_DIR}/data/ARD_simple-test_CauldronOutputDir/PressureAndTemperature_Results.HDF COPYONLY)


configure_file(test/data/ARD_simple-test_CauldronOutputDir/Time_0.000000.h5 ${CMAKE_CURRENT_BINARY_DIR}/data/ARD_simple-test_CauldronOutputDir/Time_0.000000.h5 COPYONLY)

configure_file(test/data/ARD_simple-test_CauldronOutputDir/Time_15.000000.h5 ${CMAKE_CURRENT_BINARY_DIR}/data/ARD_simple-test_CauldronOutputDir/Time_15.000000.h5 COPYONLY)

configure_file(test/data/ARD_simple-test_CauldronOutputDir/Time_40.000000.h5 ${CMAKE_CURRENT_BINARY_DIR}/data/ARD_simple-test_CauldronOutputDir/Time_40.000000.h5 COPYONLY)

configure_file(test/data/ARD_simple-test_CauldronOutputDir/Time_50.000000.h5 ${CMAKE_CURRENT_BINARY_DIR}/data/ARD_simple-test_CauldronOutputDir/Time_50.000000.h5 COPYONLY)

configure_file(test/data/ARD_simple-test_CauldronOutputDir/Time_70.000000.h5 ${CMAKE_CURRENT_BINARY_DIR}/data/ARD_simple-test_CauldronOutputDir/Time_70.000000.h5 COPYONLY)

configure_file(test/data/Project-1d.project3d ${CMAKE_CURRENT_BINARY_DIR}/data/Project-1d.project3d COPYONLY)


add_custom_command(TARGET ${LIB_NAME} PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   test/data/ARD_simple-test_CauldronOutputDir ${CMAKE_CURRENT_BINARY_DIR}/data/ARD_simple-test_CauldronOutputDir )

add_gtest( NAME CauldronIO::Project
           SOURCES test/Project.cpp test/DataStore.cpp test/VerifyData.cpp  test/VerifyData1D.cpp test/CellCentered.cpp
           LIBRARIES VisualizationIO DataModel DataAccess utilities SerialDataAccess FileSystem TableIO ${LIB_NAME} Serial_Hdf5 CBMGenerics ${HDF5_LIBRARIES} ${Boost_LIBRARIES}
           FOLDER "${BASE_FOLDER}/${LIB_NAME}" )

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
