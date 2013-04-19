#
#
#

set(target_name 123key)


file(GLOB SOURCE_FILES "${BB_ROOT}/${target_name}/src/*.C")
file(GLOB HEADER_FILES "${BB_ROOT}/${target_name}/src/*.h")

include_directories(${PROJECT_BINARY_DIR}/TableIO
  ${BB_ROOT}/../../libraries/TableIO/src
)

add_executable(${target_name} ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(${target_name} TableIO)

install(TARGETS ${target_name}
        RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)

# end of file