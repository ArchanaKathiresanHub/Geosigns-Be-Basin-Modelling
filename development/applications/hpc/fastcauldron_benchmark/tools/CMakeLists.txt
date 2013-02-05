
add_executable(remove_unnecessary_whitespace
    remove_unnecessary_whitespace.c
)


#install(TARGETS remove_unnecessary_whitespace
#    RUNTIME DESTINATION bin
#    LIBRARY DESTINATION lib
#)

#install(PROGRAMS check_project project_diff project_rm_table
#    DESTINATION bin
#)

add_executable(benchmark-set-project-parameter
  benchmark-set-project-parameter.C
  basinmodelparameter.C
)

target_link_libraries(benchmark-set-project-parameter
  utilities
  DataAccess
  TableIO
  SerialDataAccess
)

add_executable(benchmark-get-project-parameter
  benchmark-get-project-parameter.C
  basinmodelparameter.C
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

add_test( NAME hpc.benchmark.benchmark-get-project-parameter.explicit
     WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
     COMMAND bash -c "test \"`$<TARGET_FILE:benchmark-get-project-parameter> ${CMAKE_CURRENT_SOURCE_DIR}/test.project3d 'RunOptionsIoTbl . CompactionAlgorithm : string . 0'`\" = 'Effective Stress'"
)

add_test( NAME hpc.benchmark.benchmark-get-project-parameter.implicit
     WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
     COMMAND bash -c "test \"`$<TARGET_FILE:benchmark-get-project-parameter> ${CMAKE_CURRENT_SOURCE_DIR}/test.project3d  'LithotypeIoTbl . Lithotype : string .[Density : double = 3200]'`\" = 'Astheno. Mantle'"
)

add_test( NAME hpc.benchmark.benchmark-set-project-parameter.explicit
     WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
     COMMAND bash -c "$<TARGET_FILE:benchmark-set-project-parameter> ${CMAKE_CURRENT_SOURCE_DIR}/test.project3d hpc.benchmark.benchmark-set-project-parameter.explicit.project3d 'RunOptionsIoTbl . CompactionAlgorithm : string . 0' 'Another algorithm' ; test \"`$<TARGET_FILE:benchmark-get-project-parameter> hpc.benchmark.benchmark-set-project-parameter.explicit.project3d 'RunOptionsIoTbl . CompactionAlgorithm : string . 0'`\" = 'Another algorithm'"
)

add_test( NAME hpc.benchmark.benchmark-set-project-parameter.implicit
     WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
     COMMAND bash -c "$<TARGET_FILE:benchmark-set-project-parameter> ${CMAKE_CURRENT_SOURCE_DIR}/test.project3d hpc.benchmark.benchmark-set-project-parameter.implicit.project3d 'LithotypeIoTbl . Lithotype : string .[Density : double = 3200]' 'Some stone' ; test \"`$<TARGET_FILE:benchmark-get-project-parameter> hpc.benchmark.benchmark-set-project-parameter.implicit.project3d 'LithotypeIoTbl . Lithotype : string .[Density : double = 3200]'`\" = 'Some stone'"
)


