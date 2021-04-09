add_casawizard_common_gtest( NAME WorkspaceGenerator
                             SOURCES testWorkspaceGenerator.cpp )
add_casawizard_common_gtest( NAME InfoGenerator
                             SOURCES testInfoGenerator.cpp )
add_casawizard_common_gtest( NAME CMBProjectWriter
                             SOURCES testCMBProjectWriter.cpp )
add_casawizard_common_gtest( NAME ProjectTXTManipulator
                             SOURCES testProjectTXTManipulator.cpp )

configure_file( infoExpected.txt infoExpected.txt COPYONLY)
configure_file( ProjectExpected.txt ProjectExpected.txt COPYONLY)
configure_file( Project.txt Project.txt COPYONLY)

configure_file( SAC/Project.project3d SAC/Project.project3d COPYONLY )
configure_file( SAC/Inputs.HDF SAC/Inputs.HDF COPYONLY )
configure_file( SAC/Ameland_Demo_data_data2Wells.xlsx SAC/Ameland_Demo_data_data2Wells.xlsx COPYONLY )
configure_file( SAC/sacScript.casa SAC/sacScript.casa COPYONLY )
configure_file( SAC/Input.HDF SAC/Input.HDF COPYONLY )

configure_file( CMBProjectWriterTest.project3d CMBProjectWriterTest.project3d COPYONLY )
configure_file( Inputs.HDF Inputs.HDF COPYONLY )

configure_file( export/Project.project3d export/Project.project3d COPYONLY )
configure_file( export/Inputs.HDF export/Inputs.HDF COPYONLY )
configure_file( export/CalibratedInputs.HDF export/CalibratedInputs.HDF COPYONLY )
