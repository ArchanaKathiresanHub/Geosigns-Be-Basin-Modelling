
add_executable(remove_unnecessary_whitespace
    remove_unnecessary_whitespace.c
)

install(TARGETS remove_unnecessary_whitespace
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
)

install(PROGRAMS check_project project_diff project_rm_table
    DESTINATION bin
)
