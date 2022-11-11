add_casawizard_common_gtest( NAME DataFileParser
                             SOURCES testDataFileParser.cpp )
configure_file( testDataFileParserDouble.dat testDataFileParserDouble.dat COPYONLY )
configure_file( testDataFileParserDoubleHeaders.dat testDataFileParserDoubleHeaders.dat COPYONLY )
configure_file( testDataFileParserDoubleHeaders.dat testDataFileParserHeadersWrongFormat.dat COPYONLY )
configure_file( testDataFileParserInt.dat testDataFileParserInt.dat COPYONLY )
configure_file( testDataFileParserNonMatrixFormat.dat testDataFileParserNonMatrixFormat.dat COPYONLY )

add_casawizard_common_gtest( NAME ExtractWellDataXlsx
                             SOURCES testExtractWellDataXlsx.cpp wellTestData.h )
add_casawizard_common_gtest( NAME CalibrationTargetCreator
                             SOURCES testCalibrationTargetCreator.cpp )
configure_file( testWellData.xlsx testWellData.xlsx COPYONLY )
configure_file( testWellDataWithDeletes.xlsx testWellDataWithDeletes.xlsx COPYONLY )

add_casawizard_common_gtest( NAME CMBProjectReader
                             SOURCES testCMBProjectReader.cpp )
configure_file( CMBProjectReaderTest.project3d CMBProjectReaderTest.project3d COPYONLY )
configure_file( HeatMap.hdf HeatMap.hdf COPYONLY )

add_casawizard_common_gtest( NAME MapReader
                             SOURCES testMapReader.cpp )
configure_file( MapReaderTest.project3d MapReaderTest.project3d COPYONLY )
configure_file( MapReaderTestWindowed.project3d MapReaderTestWindowed.project3d COPYONLY )
configure_file( Inputs.HDF Inputs.HDF COPYONLY )

add_casawizard_common_gtest( NAME ExtractWellDataLAS
                             SOURCES TestExtractWellDataLAS.cpp )
configure_file( Test1.las Test1.las COPYONLY )
configure_file( testWrap.las testWrap.las COPYONLY )
configure_file( Version3.las Version3.las COPYONLY )
configure_file( specialCharactersInWellName.las specialCharactersInWellName.las COPYONLY )

add_casawizard_common_gtest( NAME LASWellInfoSectionReader
                             SOURCES testLASWellInfoSectionReader.cpp )

add_casawizard_common_gtest( NAME LASDataSectionReader
                             SOURCES testLASDataSectionReader.cpp )
add_casawizard_common_gtest( NAME LASCurveInfoSectionReader
                             SOURCES testLASCurveInfoSectionReader.cpp )
add_casawizard_common_gtest( NAME LASVersionInfoSectionReader
                             SOURCES testLASVersionInfoSectionReader.cpp )

add_casawizard_common_gtest( NAME VSETReader
                             SOURCES testVSETReader.cpp )
configure_file( VSETdata.vs VSETdata.vs COPYONLY )
