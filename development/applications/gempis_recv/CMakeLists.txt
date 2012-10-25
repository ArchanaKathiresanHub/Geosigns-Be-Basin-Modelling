add_executable(gempis_recv
	src/gempis_recv.C
)

target_link_libraries(gempis_recv
	pthread
	rt
)

install(TARGETS gempis_recv
	RUNTIME DESTINATION bin
)
