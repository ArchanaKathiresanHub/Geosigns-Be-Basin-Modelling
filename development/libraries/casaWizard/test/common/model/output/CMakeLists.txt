add_casawizard_common_gtest( NAME WorkspaceGenerator
                             SOURCES testWorkspaceGenerator.cpp )


configure_file( SAC/Project.project3d SAC/Project.project3d COPYONLY )
configure_file( SAC/Inputs.HDF SAC/Inputs.HDF COPYONLY )
configure_file( SAC/Ameland_Demo_data_data2Wells.xlsx SAC/Ameland_Demo_data_data2Wells.xlsx COPYONLY )
configure_file( SAC/sacScript.casa SAC/sacScript.casa COPYONLY )
configure_file( SAC/Input.HDF SAC/Input.HDF COPYONLY )
