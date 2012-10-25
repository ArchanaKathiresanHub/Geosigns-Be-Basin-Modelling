
add_library(ThreadUtilities
	src/Barrier.C
	src/Mutex.C
	src/Rendezvous.C
)

install(TARGETS ThreadUtilities	
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)

