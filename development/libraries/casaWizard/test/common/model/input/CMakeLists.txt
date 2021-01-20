add_casawizard_common_gtest( NAME DataFileParser
                             SOURCES testDataFileParser.cpp )
configure_file( testDataFileParserDouble.dat testDataFileParserDouble.dat COPYONLY )
configure_file( testDataFileParserInt.dat testDataFileParserInt.dat COPYONLY )

add_casawizard_common_gtest( NAME ExtractWellDataXlsx
                             SOURCES testExtractWellDataXlsx.cpp wellTestData.h )
add_casawizard_common_gtest( NAME CalibrationTargetCreator
                             SOURCES testCalibrationTargetCreator.cpp )
configure_file( testWellData.xlsx testWellData.xlsx COPYONLY )
configure_file( testWellDataWithDeletes.xlsx testWellDataWithDeletes.xlsx COPYONLY )

add_casawizard_common_gtest( NAME XlsxFileMapCreator
                             SOURCES testXlsxFileMapCreator.cpp )

add_casawizard_common_gtest( NAME CMBProjectReader
                             SOURCES testCMBProjectReader.cpp )
configure_file( CMBProjectReaderTest.project3d CMBProjectReaderTest.project3d COPYONLY )

add_casawizard_common_gtest( NAME MapReader
                             SOURCES testMapReader.cpp )
configure_file( MapReaderTest.project3d MapReaderTest.project3d COPYONLY )
configure_file( Inputs.HDF Inputs.HDF COPYONLY )