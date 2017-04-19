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
# Building the Project3dAPI library
#
# ATTENTION: The file is used by Basin Modeling and Geocase.
#
set(allheaders
   ${CMAKE_CURRENT_BINARY_DIR}/bmAPI.h
   ${CMAKE_CURRENT_BINARY_DIR}/ProjectIoAPI.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableProjectIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableProjectNotesIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableRelatedProjectsIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableIoOptionsIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableRunOptionsIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableSnapshotIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableSensAnalysisIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableWellLocIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableTouchstoneWellIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableTouchstoneMapIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableGridMapIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableStratIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableSourceRockLithoIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableSourceRockPropIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableMobLayThicknIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableAllochthonLithoIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableAllochthonLithoDistribIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableAllochthonLithoInterpIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableCrustIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableContCrustalThicknessIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableBasaltThicknessIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableBasementIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableFormPressIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableFormTempIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableDisplayContourIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableSurfaceTempIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TablePermafrostIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableGeologicalBoundaryIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TablePropertyBoundaryIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableBoundaryValuesIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableSurfaceDepthIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableMntlHeatFlowIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableRawBhtIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableRawBhtBitsizeIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableMarsdenCoefIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableVrmIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableSonicIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableBulkDensityIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableBiomarkermIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableBiomarkerKinIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableSmectiteIlliteIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableSmectiteIlliteKinIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableReservoirIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableReservoirPropIoTbl.h
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
   ${CMAKE_CURRENT_BINARY_DIR}/TableFtGrainIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableFtLengthIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableFtSampleIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableFtPredLengthCountsHistIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableFtPredLengthCountsHistDataIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableFtClWeightPercBinsIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableLithotypeIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableLitThCondIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableLitHeatCapIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableFluidtypeIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableFltThCondIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableFltHeatCapIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableFltDensityIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableFltViscoIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableFilterDepthIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableFilterTimeIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableFilterTimeDepthIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableDepthIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableTimeIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/Table3DTimeIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/Table1DTimeIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableTemperatureIsoIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableVrIsoIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableTwoWayTimeIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableGenexHistoryLocationIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableCTCIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableLangmuirAdsorptionCapacityIsothermSetIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableLangmuirAdsorptionCapacityTOCFunctionIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableAdsorptionHistoryIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableIrreducibleWaterSaturationIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableSGDensityIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableBPANameMapping.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableSimulationDetailsIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableGridIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableOutputTablesFileIoTbl.h
   ${CMAKE_CURRENT_BINARY_DIR}/TableOutputTablesIoTbl.h
)

set(allsources
   ${CMAKE_CURRENT_BINARY_DIR}/ProjectIoAPI.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableProjectIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableProjectNotesIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableRelatedProjectsIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableIoOptionsIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableRunOptionsIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableSnapshotIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableSensAnalysisIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableWellLocIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableTouchstoneWellIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableTouchstoneMapIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableGridMapIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableStratIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableSourceRockLithoIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableSourceRockPropIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableMobLayThicknIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableAllochthonLithoIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableAllochthonLithoDistribIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableAllochthonLithoInterpIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableCrustIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableContCrustalThicknessIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableBasaltThicknessIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableBasementIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableFormPressIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableFormTempIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableDisplayContourIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableSurfaceTempIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TablePermafrostIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableGeologicalBoundaryIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TablePropertyBoundaryIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableBoundaryValuesIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableSurfaceDepthIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableMntlHeatFlowIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableRawBhtIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableRawBhtBitsizeIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableMarsdenCoefIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableVrmIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableSonicIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableBulkDensityIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableBiomarkermIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableBiomarkerKinIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableSmectiteIlliteIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableSmectiteIlliteKinIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableReservoirIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableReservoirPropIoTbl.cpp
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
   ${CMAKE_CURRENT_BINARY_DIR}/TableFtGrainIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableFtLengthIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableFtSampleIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableFtPredLengthCountsHistIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableFtPredLengthCountsHistDataIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableFtClWeightPercBinsIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableLithotypeIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableLitThCondIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableLitHeatCapIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableFluidtypeIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableFltThCondIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableFltHeatCapIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableFltDensityIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableFltViscoIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableFilterDepthIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableFilterTimeIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableFilterTimeDepthIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableDepthIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableTimeIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/Table3DTimeIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/Table1DTimeIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableTemperatureIsoIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableVrIsoIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableTwoWayTimeIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableGenexHistoryLocationIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableCTCIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableLangmuirAdsorptionCapacityIsothermSetIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableLangmuirAdsorptionCapacityTOCFunctionIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableAdsorptionHistoryIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableIrreducibleWaterSaturationIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableSGDensityIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableBPANameMapping.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableSimulationDetailsIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableGridIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableOutputTablesFileIoTbl.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/TableOutputTablesIoTbl.cpp
)

set(generator_source_files
   ${allheaders}
   ${allsources}
)

include_directories(src ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR}/../TableIO )

add_custom_command(OUTPUT ${generator_source_files} ${CMAKE_CURRENT_BINARY_DIR}/Project3dAPI.i
   DEPENDS parspec ../TableIO/BasinModellerProperty.Spec
   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
   COMMAND parspec -schema CauldronSchema -directory ${CMAKE_CURRENT_BINARY_DIR} -input ../TableIO/BasinModellerProperty.Spec -bmapi
)

add_library(bmAPI ${generator_source_files})

# under Linux we are building Boost that is why this target exist
if (UNIX)
add_dependencies(bmAPI Boost)
endif (UNIX)

add_dependencies(bmAPI
   TableIO
   FileSystem
   utilities
)

bm_include_libraries(bmAPI
   TableIO
   FileSystem
   utilities
)

target_link_libraries(bmAPI
   TableIO
   FileSystem
   utilities
)

install(TARGETS bmAPI
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)

configure_file(
      ${CMAKE_CURRENT_SOURCE_DIR}/test/Reference.project3d
      ${CMAKE_CURRENT_BINARY_DIR}
      COPYONLY
)

add_gtest(NAME TableIO::CauldronSchemaAPITest
          SOURCES test/CauldronSchemaAPITest.cpp ${allheaders} test/CauldronSchemaAPITest.h
          LIBRARIES bmAPI TableIO utilities FileSystem
)

if (MSVC)
   ##### Generate the C# API
   configure_file(src/version.rc.cmake version.rc)
   configure_file(src/AssemblyInfo.cs.cmake csharp/Properties/AssemblyInfo.cs)
   
   set(CMAKE_SWIG_OUTDIR ${CMAKE_CURRENT_BINARY_DIR}/csharp)

   set_source_files_properties( ${CMAKE_CURRENT_BINARY_DIR}/Project3dAPI.i
      PROPERTIES SWIG_FLAGS "-namespace;Shell.BasinModeling.Project3dAPI;-I${CMAKE_CURRENT_BINARY_DIR}"
      CPLUSPLUS ON
   )

   swig_add_module(Project3dAPI csharp ${CMAKE_CURRENT_BINARY_DIR}/Project3dAPI.i
      ${CMAKE_CURRENT_BINARY_DIR}/version.rc
   )

   swig_link_libraries(Project3dAPI bmAPI TableIO FileSystem ${Boost_LIBRARIES})

   # Before C# generation, remove all existing files. The directory should be empty before generation	
   add_custom_command( TARGET Project3dAPI
      PRE_BUILD
      COMMAND ${CMAKE_COMMAND} ARGS "-E" "remove" "*.cs"
      WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/csharp"
   )

   # Visual studio should reload the project after C# generation
   add_custom_command(TARGET Project3dAPI
      PRE_LINK 
      COMMAND ${CMAKE_COMMAND} ARGS "-E" "touch_nocreate" "Shell.BasinModeling.Project3dAPI.csproj"
      WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/csharp"
   )

   # Generate C# project to compile generated C# files. 
   new_guid(Guid)
   configure_file( Shell.BasinModeling.Project3dAPI.csproj.cmake ${CMAKE_CURRENT_BINARY_DIR}/csharp/read-only/Shell.BasinModeling.Project3dAPI.csproj @ONLY)
   
   # TFS checksout files in 'Read only' mode. configure_file retains this mode, however we want to 'touch' the file when the C# files have been generated
   # So copy it, with the right permissions.
   file(COPY ${CMAKE_CURRENT_BINARY_DIR}/csharp/read-only/Shell.BasinModeling.Project3dAPI.csproj
      DESTINATION  ${CMAKE_CURRENT_BINARY_DIR}/csharp
      FILE_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ
   )

   include_external_msproject(Shell.BasinModeling.Project3dAPI ${CMAKE_CURRENT_BINARY_DIR}/csharp/Shell.BasinModeling.Project3dAPI.csproj
      TYPE "{FAE04EC0-301F-11D3-BF4B-00C04F79EFBC}" # This GUID is a Windows C# project (see also http://msdn.microsoft.com/en-us/library/hb23x61k(v=vs.80).aspx ) 
      PLATFORM "${BM_WINDOWS_PLATFORM}"
      GUID "${Guid}"
      Project3dAPI
   )
    
   ###### Installation
   install(TARGETS Project3dAPI
      RUNTIME DESTINATION bin
      LIBRARY DESTINATION bin
   )
    
   install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/csharp/Debug/Shell.BasinModeling.Project3dAPI.dll
      DESTINATION	bin
      CONFIGURATIONS Debug
   )

   install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/csharp/Release/Shell.BasinModeling.Project3dAPI.dll
      DESTINATION	bin
      CONFIGURATIONS Release
   )

   ###### Compile and run unit tests	
   set(CSHARP_UNIT_TESTS_SRC
      test/Project3dAPITest.cs test/Project3dAPITest_gen.cs
   )

   configure_file(test/AssemblyInfo.cs.cmake csharp-test/Properties/AssemblyInfo.cs)

   add_csharp_unittest(NAME Shell.BasinModeling.Project3dAPI.Test
      PLATFORM ${BM_WINDOWS_PLATFORM}
      DIRECTORY csharp-test
      TEST_SOURCES ${CSHARP_UNIT_TESTS_SRC}
      DEPENDS Shell.BasinModeling.Project3dAPI
      TESTLIST "Test"
      TFS_SERVER_URL ${BM_TFS_SERVER_URL}
      TFS_BUILD_NUMBER ${BM_TFS_BUILD_NUMBER}
      TFS_PROJECT_NAME ${BM_TFS_PROJECT_NAME}
      DEPLOYMENT_ITEMS "../\$(Configuration)/Project3dAPI.dll" 
      PROJECT_REFERENCE Shell.BasinModeling.Project3dAPI
      "${Guid}"
      "${CMAKE_CURRENT_BINARY_DIR}/csharp/Shell.BasinModeling.Project3dAPI.csproj"
   )
   
endif()
 
