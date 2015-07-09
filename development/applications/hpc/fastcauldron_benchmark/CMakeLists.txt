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

include_directories(
      tools
      ${CMAKE_SOURCE_DIR}/applications/xcauldron/src
      ${HDF5_INCLUDE_DIRS}
)

add_executable(benchmark
  tools/benchmark.C
  tools/fastcauldronenvironment.C
  tools/FastCauldronEnvironmentConfigurationTokenizer.C
  tools/cmdlineparameter.C
  tools/project3dparameter.C
  tools/parameterdefinitions.C
  tools/parametersettings.C
  tools/parser.C
  tools/system.C
  tools/hpccluster.C
  tools/configurationfile.C
  tools/resultstabulator.C
  tools/variabledefinitions.C
)

target_link_libraries(benchmark
  utilities
  DataModel
  DataAccess
  TableIO
  SerialDataAccess
  projectdependencies
)


if (BM_INSTALL_BENCHMARK_SUITE)
   
   install(TARGETS benchmark
         RUNTIME DESTINATION fcbench
   )

   install(DIRECTORY input 
       DESTINATION fcbench/
       USE_SOURCE_PERMISSIONS
   )

   install(DIRECTORY config
       DESTINATION fcbench/
       USE_SOURCE_PERMISSIONS
   )

endif()

bm_include_libraries(
      utilities
      DataModel
      DataAccess
      TableIO
      SerialDataAccess
)

add_gtest(
   NAME    hpc::FastCauldronEnvironmentConfigurationTokenizer
   SOURCES test/FastCauldronEnvironmentConfigurationTokenizer.C
           tools/FastCauldronEnvironmentConfigurationTokenizer.C
           tools/system.C
   LIBRARIES
           utilities
           projectdependencies
           DataAccess
           SerialDataAccess
)

add_gtest(
   NAME    hpc::FastCauldronEnvironment
   SOURCES test/FastcauldronEnvironment.C
           tools/fastcauldronenvironment.C
           tools/FastCauldronEnvironmentConfigurationTokenizer.C
           tools/system.C
   LIBRARIES
           utilities
           projectdependencies
           DataAccess
           SerialDataAccess
)

add_gtest(
   NAME    hpc::Project3DParameter
   SOURCES test/Project3DParameter.C
           tools/project3dparameter.C
           tools/parser.C
           test/test.project3d.C
   LIBRARIES
           utilities
           DataAccess
           SerialDataAccess
)           


# Currently disabled, because this test takes 2 minutes, which is
# uncomfortably long   
#add_test(NAME hpc.benchmark.input_wellformedness_tests
#    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
#    COMMAND /bin/bash "${CMAKE_CURRENT_SOURCE_DIR}/input.t" quiet
#)

