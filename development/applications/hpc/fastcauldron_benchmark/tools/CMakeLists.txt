
add_executable(remove_unnecessary_whitespace
    remove_unnecessary_whitespace.c
)



add_executable(benchmark
  benchmark.C
  fastcauldronenvironment.C
  cmdlineparameter.C
  project3dparameter.C
  parameterdefinitions.C
  parametersettings.C
  parser.C
  system.C
  hpccluster.C
  configurationfile.C
  resultstabulator.C
  variabledefinitions.C
)

target_link_libraries(benchmark
  utilities
  DataAccess
  TableIO
  SerialDataAccess
  projectdependencies
)

add_executable(benchmark-set-project-parameter
  benchmark-set-project-parameter.C
  project3dparameter.C
  parser.C
)

target_link_libraries(benchmark-set-project-parameter
  utilities
  DataAccess
  TableIO
  SerialDataAccess
)

add_executable(benchmark-get-project-parameter
  benchmark-get-project-parameter.C
  project3dparameter.C
  parser.C
)

target_link_libraries(benchmark-get-project-parameter
  utilities
  DataAccess
  TableIO
  SerialDataAccess
)

include_directories(
    ${HDF5_serial_INCLUDE_DIRS}
)


bm_include_libraries(
      utilities
      DataAccess
      TableIO
      SerialDataAccess
)
include_directories(
      ${CMAKE_SOURCE_DIR}/applications/xcauldron/src
)

if (BM_INSTALL_BENCHMARK_SUITE)
   install(TARGETS benchmark benchmark-set-project-parameter benchmark-get-project-parameter
         RUNTIME DESTINATION fcbench
   )
   install(TARGETS remove_unnecessary_whitespace
       RUNTIME DESTINATION fcbench
   )

   install(PROGRAMS check_project project_diff project_rm_table
       DESTINATION fcbench
   )
endif()

add_test( NAME hpc.benchmark.benchmark-get-project-parameter.explicit.1
     WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
     COMMAND bash -c "test \"`$<TARGET_FILE:benchmark-get-project-parameter> ${CMAKE_CURRENT_SOURCE_DIR}/test.project3d 'RunOptionsIoTbl . CompactionAlgorithm : string . 0'`\" = 'Effective Stress'"
)

add_test( NAME hpc.benchmark.benchmark-get-project-parameter.explicit.2
     WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
     COMMAND bash -c "test \"`$<TARGET_FILE:benchmark-get-project-parameter> ${CMAKE_CURRENT_SOURCE_DIR}/test.project3d 'StratIoTbl .  ElementRefinementZ : int . *'`\" = '1'"
)

add_test( NAME hpc.benchmark.benchmark-get-project-parameter.implicit
     WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
     COMMAND bash -c "test \"`$<TARGET_FILE:benchmark-get-project-parameter> ${CMAKE_CURRENT_SOURCE_DIR}/test.project3d  'LithotypeIoTbl . Lithotype : string .[Density : double = 3200]'`\" = 'Astheno. Mantle'"
)

add_test( NAME hpc.benchmark.benchmark-get-project-parameter.choice.1
     WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
     COMMAND bash -c "test \"`$<TARGET_FILE:benchmark-get-project-parameter> ${CMAKE_CURRENT_SOURCE_DIR}/test.project3d 'RunOptionsIoTbl . TempDiffBasedStepping : int . 0 { BurialRateTimeStepping = 0, TempDiffTimeStepping = 1 }'`\" = 'BurialRateTimeStepping'"
)

add_test( NAME hpc.benchmark.benchmark-set-project-parameter.explicit.1
     WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
     COMMAND bash -c "$<TARGET_FILE:benchmark-set-project-parameter> ${CMAKE_CURRENT_SOURCE_DIR}/test.project3d hpc.benchmark.benchmark-set-project-parameter.explicit.1.project3d 'RunOptionsIoTbl . CompactionAlgorithm : string . 0' 'Another algorithm' ; test \"`$<TARGET_FILE:benchmark-get-project-parameter> hpc.benchmark.benchmark-set-project-parameter.explicit.1.project3d 'RunOptionsIoTbl . CompactionAlgorithm : string . 0'`\" = 'Another algorithm'"
)

add_test( NAME hpc.benchmark.benchmark-set-project-parameter.explicit.2
     WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
     COMMAND bash -c "$<TARGET_FILE:benchmark-set-project-parameter> ${CMAKE_CURRENT_SOURCE_DIR}/test.project3d hpc.benchmark.benchmark-set-project-parameter.explicit.2.project3d 'StratIoTbl.ElementRefinementZ:int.*' '20' ; test \"`$<TARGET_FILE:benchmark-get-project-parameter> hpc.benchmark.benchmark-set-project-parameter.explicit.2.project3d 'StratIoTbl.ElementRefinementZ:int.*'`\" = 20"
)

add_test( NAME hpc.benchmark.benchmark-set-project-parameter.implicit
     WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
     COMMAND bash -c "$<TARGET_FILE:benchmark-set-project-parameter> ${CMAKE_CURRENT_SOURCE_DIR}/test.project3d hpc.benchmark.benchmark-set-project-parameter.implicit.project3d 'LithotypeIoTbl . Lithotype : string .[Density : double = 3200]' 'Some stone' ; test \"`$<TARGET_FILE:benchmark-get-project-parameter> hpc.benchmark.benchmark-set-project-parameter.implicit.project3d 'LithotypeIoTbl . Lithotype : string .[Density : double = 3200]'`\" = 'Some stone'"
)

add_test( NAME hpc.benchmark.benchmark-set-project-parameter.choice.1
     WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
     COMMAND bash -c "$<TARGET_FILE:benchmark-set-project-parameter> ${CMAKE_CURRENT_SOURCE_DIR}/test.project3d hpc.benchmark.benchmark-set-project-parameter.choice.1.project3d 'RunOptionsIoTbl . TempDiffBasedStepping : int . 0 { BurialRateTimeStepping = 0, TempDiffTimeStepping = 1 }' 'TempDiffTimeStepping'; test \"`$<TARGET_FILE:benchmark-get-project-parameter> hpc.benchmark.benchmark-set-project-parameter.choice.1.project3d 'RunOptionsIoTbl . TempDiffBasedStepping : int. 0 { BurialRateTimeStepping = 0, TempDiffTimeStepping = 1}'`\" = TempDiffTimeStepping " 
)

