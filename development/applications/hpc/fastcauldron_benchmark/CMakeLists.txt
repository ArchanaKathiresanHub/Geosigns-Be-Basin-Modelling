
enable_testing()

add_subdirectory(tools)

#install(DIRECTORY input 
#    DESTINATION share/fcbench
#    PATTERN "*"
#    PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
#                GROUP_EXECUTE GROUP_READ
#		WORLD_EXECUTE WORLD_READ
#)

add_test(NAME hpc.benchmark.input_wellformedness_tests
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/input.t" quiet
)

