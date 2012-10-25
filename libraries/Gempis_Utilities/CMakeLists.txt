add_library(Gempis_Utilities
	src/Gempis.C
)

install(TARGETS Gempis_Utilities
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)



