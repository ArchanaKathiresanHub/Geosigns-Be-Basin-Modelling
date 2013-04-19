add_executable(gempis_send
	src/gempis_send.C
)

install(TARGETS gempis_send
	RUNTIME DESTINATION bin
)
