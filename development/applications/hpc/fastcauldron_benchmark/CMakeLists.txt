
add_subdirectory(tools)

if (BM_INSTALL_BENCHMARK_SUITE)
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
      DataAccess
      TableIO
      SerialDataAccess
)

include_directories(
      tools
      ${CMAKE_SOURCE_DIR}/applications/xcauldron/src
      ${HDF5_serial_INCLUDE_DIRS}
)

add_executable(TestFastCauldronEnvironment
   test/FastcauldronEnvironment.C
   tools/fastcauldronenvironment.C
   tools/system.C
)

target_link_libraries(TestFastCauldronEnvironment
      utilities
      projectdependencies
      DataAccess
      SerialDataAccess
)      


set_target_properties(TestFastCauldronEnvironment
      PROPERTIES COMPILE_FLAGS "-Dprivate=public"
)

add_test(NAME hpc::FastCauldronEnvironment::Configuration::Tokenizer
      COMMAND TestFastCauldronEnvironment FastCauldronEnvironment::Configuration::Tokenizer
)

add_test(NAME hpc::FastCauldronEnvironment::Configuration::readTemplates
      COMMAND TestFastCauldronEnvironment FastCauldronEnvironment::Configuration::readTemplates
)

add_test(NAME hpc::FastCauldronEnvironment::Configuration::getRunScript
      COMMAND TestFastCauldronEnvironment FastCauldronEnvironment::Configuration::getRunScript
)

add_test(NAME hpc::FastCauldronEnvironment::jobHasRanBefore
      COMMAND TestFastCauldronEnvironment FastCauldronEnvironment::jobHasRanBefore
)

# Currently disabled, because this test takes 2 minutes, which is
# uncomfortably long   
#add_test(NAME hpc.benchmark.input_wellformedness_tests
#    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
#    COMMAND /bin/bash "${CMAKE_CURRENT_SOURCE_DIR}/input.t" quiet
#)

