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
  tools/benchmark.cpp
  tools/fastcauldronenvironment.cpp
  tools/FastCauldronEnvironmentConfigurationTokenizer.cpp
  tools/cmdlineparameter.cpp
  tools/project3dparameter.cpp
  tools/parameterdefinitions.cpp
  tools/parametersettings.cpp
  tools/parser.cpp
  tools/system.cpp
  tools/hpccluster.cpp
  tools/configurationfile.cpp
  tools/resultstabulator.cpp
  tools/variabledefinitions.cpp
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

add_gtest(
   NAME    hpc::FastCauldronEnvironmentConfigurationTokenizer
   SOURCES test/FastCauldronEnvironmentConfigurationTokenizer.cpp
           tools/FastCauldronEnvironmentConfigurationTokenizer.cpp
           tools/system.cpp
   LIBRARIES
           utilities
           projectdependencies
           DataAccess
           SerialDataAccess
)

add_gtest(
   NAME    hpc::FastCauldronEnvironment
   SOURCES test/FastcauldronEnvironment.cpp
           tools/fastcauldronenvironment.cpp
           tools/FastCauldronEnvironmentConfigurationTokenizer.cpp
           tools/system.cpp
   LIBRARIES
           utilities
           projectdependencies
           DataAccess
           SerialDataAccess
)

add_gtest(
   NAME    hpc::Project3DParameter
   SOURCES test/Project3DParameter.cpp
           tools/project3dparameter.cpp
           tools/parser.cpp
           test/test.project3d.cpp
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

