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

set( LIB_NAME "VisualizationIO_projectHandle" )

include_directories( SYSTEM
   ${HDF5_INCLUDE_DIRS}
   ${PUGIXML_INCLUDE_DIR}
   ${LZ_INCLUDE_DIR}
)

create_bm_library( TARGET ${LIB_NAME}
                   LIBRARIES
                             VisualizationIO
                             SerialDataAccess
                             DataAccess
                             TableIO
                             FileSystem 
                             DataModel
                             utilities
                             PugiXMLlib
                             LZlib
                             ${HDF5_LIBRARIES}
                             ${Boost_LIBRARIES} )

# copy some unit test data
copy_test_file(data/ARD_simple-test.project3d)
copy_test_file(data/Inputs.HDF)
copy_test_file(data/ARD_simple-test_MassBalance)
copy_test_file(data/ARD_simple-test_CauldronOutputDir/Genex5_Results.HDF)
copy_test_file(data/ARD_simple-test_CauldronOutputDir/History_ARD_simple-test_genex_sr_5500_4500.dat)
copy_test_file(data/ARD_simple-test_CauldronOutputDir/HistoryGenex.txt)
copy_test_file(data/ARD_simple-test_CauldronOutputDir/MyWell.BHF)
copy_test_file(data/ARD_simple-test_CauldronOutputDir/HighResDecompaction_Results.HDF)
copy_test_file(data/ARD_simple-test_CauldronOutputDir/HighResMigration_Results.HDF)
copy_test_file(data/ARD_simple-test_CauldronOutputDir/PressureAndTemperature_Results.HDF)

copy_test_file(data/ARD_simple-test_CauldronOutputDir/Time_0.000000.h5)
copy_test_file(data/ARD_simple-test_CauldronOutputDir/Time_15.000000.h5)
copy_test_file(data/ARD_simple-test_CauldronOutputDir/Time_40.000000.h5)
copy_test_file(data/ARD_simple-test_CauldronOutputDir/Time_50.000000.h5)
copy_test_file(data/ARD_simple-test_CauldronOutputDir/Time_70.000000.h5)
copy_test_file(data/Project-1d.project3d)

add_custom_command(TARGET ${LIB_NAME} PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   test/data/ARD_simple-test_CauldronOutputDir ${CMAKE_CURRENT_BINARY_DIR}/data/ARD_simple-test_CauldronOutputDir )

add_gtest( NAME CauldronIO::Project
           SOURCES test/Project.cpp test/DataStore.cpp test/VerifyData.cpp  test/VerifyData1D.cpp test/CellCentered.cpp
           LIBRARIES VisualizationIO DataModel DataAccess utilities SerialDataAccess FileSystem TableIO ${LIB_NAME} Serial_Hdf5 CBMGenerics ${HDF5_LIBRARIES} ${Boost_LIBRARIES}
           FOLDER "${BASE_FOLDER}/${LIB_NAME}" )
