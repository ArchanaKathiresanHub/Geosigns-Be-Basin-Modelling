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
# Building the Project3dAPI library
#
# ATTENTION: The file is used by Basin Modeling and Geocase.
#
set(allheaders
   ${CMAKE_CURRENT_BINARY_DIR}/Project3dAPI.h
   ${CMAKE_CURRENT_BINARY_DIR}/ProjectIoAPI.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableProjectIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableRelatedProjectsIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableRunOptionsIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableUserDefinedSnapshotIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableSnapshotIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableWellLocIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableGridMapIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableStratIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableSourceRockLithoIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableMobLayThicknIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableAllochthonLithoIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableAllochthonLithoDistribIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableAllochthonLithoInterpIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableCrustIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableContCrustalThicknessIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableOceaCrustalThicknessIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableBasaltThicknessIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableBasementIoTbl.h   
   ${CMAKE_CURRENT_BINARY_DIR}/TableSurfaceTempIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TablePermafrostIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableGeologicalBoundaryIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TablePropertyBoundaryIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableBoundaryValuesIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableSurfaceDepthIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableMntlHeatFlowIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableReservoirIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableReservoirOptionsIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableBioDegradIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableDiffusionIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TablePalinspasticIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableFaultcutIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TablePressureFaultcutIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TablePressureFuncIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableTrapIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableMigrationIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableTrapperIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableDataMiningIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableLithotypeIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableLitThCondIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableLitHeatCapIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableFluidtypeIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableFltThCondIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableFltHeatCapIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableFilterTimeIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableTimeIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/Table3DTimeIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableTwoWayTimeIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableGenexHistoryLocationIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableCTCIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableCTCRiftingHistoryIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableLangmuirAdsorptionCapacityIsothermSetIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableLangmuirAdsorptionCapacityTOCFunctionIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableAdsorptionHistoryIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableIrreducibleWaterSaturationIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableSGDensityIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableBPANameMapping.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableSimulationDetailsIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableOutputTablesFileIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableOutputTablesIoTbl.h

)

set(allsources
   ${CMAKE_CURRENT_BINARY_DIR}/ProjectIoAPI.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableProjectIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableRelatedProjectsIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableRunOptionsIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableUserDefinedSnapshotIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableSnapshotIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableWellLocIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableGridMapIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableStratIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableSourceRockLithoIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableMobLayThicknIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableAllochthonLithoIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableAllochthonLithoDistribIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableAllochthonLithoInterpIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableCrustIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableContCrustalThicknessIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableOceaCrustalThicknessIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableBasaltThicknessIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableBasementIoTbl.cpp   
   ${CMAKE_CURRENT_BINARY_DIR}/TableSurfaceTempIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TablePermafrostIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableGeologicalBoundaryIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TablePropertyBoundaryIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableBoundaryValuesIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableSurfaceDepthIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableMntlHeatFlowIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableReservoirIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableReservoirOptionsIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableBioDegradIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableDiffusionIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TablePalinspasticIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableFaultcutIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TablePressureFaultcutIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TablePressureFuncIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableTrapIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableMigrationIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableTrapperIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableDataMiningIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableLithotypeIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableLitThCondIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableLitHeatCapIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableFluidtypeIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableFltThCondIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableFltHeatCapIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableFilterTimeIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableTimeIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/Table3DTimeIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableTwoWayTimeIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableGenexHistoryLocationIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableCTCIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableCTCRiftingHistoryIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableLangmuirAdsorptionCapacityIsothermSetIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableLangmuirAdsorptionCapacityTOCFunctionIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableAdsorptionHistoryIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableIrreducibleWaterSaturationIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableSGDensityIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableBPANameMapping.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableSimulationDetailsIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableOutputTablesFileIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableOutputTablesIoTbl.cpp
)

set(generator_source_files
   ${allheaders}
   ${allsources}
)

include_directories(src ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR}/../TableIO )

add_custom_command(OUTPUT ${generator_source_files}
   DEPENDS parspec ../TableIO/BasinModellerProperty.Spec
   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
   COMMAND parspec -schema CauldronSchema -directory ${CMAKE_CURRENT_BINARY_DIR} -input ../TableIO/BasinModellerProperty.Spec -project3dapi
)

add_custom_target(cpp_generated_api_files DEPENDS ${generator_source_files})
set_target_properties( cpp_generated_api_files PROPERTIES FOLDER "${BASE_FOLDER}/cpp_generated_api_files" )

set( LIB_NAME "project3d" )
add_library(${LIB_NAME} ${generator_source_files})
target_include_directories( ${LIB_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src" )
set_target_properties( ${LIB_NAME} PROPERTIES FOLDER "${BASE_FOLDER}/${LIB_NAME}" )

# under Linux we are building Boost that is why this target exist
if (UNIX)
add_dependencies(${LIB_NAME} Boost)
endif (UNIX)

add_dependencies(${LIB_NAME}
   cpp_generated_api_files
)

target_link_libraries(${LIB_NAME}
   TableIO
   FileSystem
   utilities
)

install(TARGETS ${LIB_NAME}
   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)

copy_test_file(Reference.project3d)

add_gtest(NAME TableIO::CauldronSchemaAPITest
          SOURCES test/CauldronSchemaAPITest.cpp test/CauldronSchemaAPITest.h
          DEPENDS cpp_generated_api_files
          LIBRARIES ${LIB_NAME} TableIO utilities FileSystem
          FOLDER "${BASE_FOLDER}/cpp_generated_api_files"
)

if (MSVC)
   generate_version_by_git_last_checkin(../TableIO/BasinModellerProperty.Spec API_FILE_GIT_DATE_AS_VER)
   MESSAGE(STATUS "Version extracted for Project3dAPI as: ${API_FILE_GIT_DATE_AS_VER}")
   ##### Generate the C# API
   generate_csharp_api(
      CSPROJ_NAME                   Project3dAPI
      CSPROJ_NAMESPACE              Bpa2.Basin.Project3d
      CSPROJ_ASSEMBLY_VERSION       ${API_FILE_GIT_DATE_AS_VER}
      # Info for Nuget package
      CSPROJ_ASSEMBLY_OWNER         "Sergey Koshelev"
      CSPROJ_ASSEMBLY_DESCRIPTION   "C# API for Cauldron Project3d file reading/writing."
      CSPROJ_ASSEMBLY_RELEASE_NOTES "Version ${API_FILE_GIT_DATE_AS_VER}. Initial nuget package release"
      CSPROJ_ASSEMBLY_TAGS          "Cauldron backend Project3d API"
      CSPROJ_ASSEMBLY_COPYRIGHT     "Copyright (C) 2012-2017"
      CSPROJ_EXTRA_DEPS             cpp_generated_api_files
      CSPROJ_LIBRARIES              project3d TableIO FileSystem ${Boost_LIBRARIES}
      CSHARP_UNIT_TESTS_SRC         test/Project3dAPITest.cs test/Project3dAPITest_gen.cs
   )
endif()

