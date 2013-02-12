
enable_testing()

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

add_test(NAME hpc.benchmark.input_wellformedness_tests
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/input.t" quiet
)

