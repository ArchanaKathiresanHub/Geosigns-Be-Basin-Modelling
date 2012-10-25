add_executable(tracktraps
	src/ProjectHandle.C
	src/PersistentTrap.C
	src/PersistentTrapFactory.C
	src/Reservoir.C
	src/Trap.C
	src/generics.C
	src/main.C
	src/utils.C
)

bm_include_libraries(
	EosPack
	DataAccess
	SerialDataAccess
	TableIO
	utilities
	CBMGenerics
)

include_directories(
	${HDF5_INCLUDE_DIRS}
)


target_link_libraries(tracktraps
	EosPack
	DataAccess
	SerialDataAccess
	TableIO
	utilities
	CBMGenerics
	${HDF5_LIBRARIES}
)

install(TARGETS tracktraps
	RUNTIME DESTINATION bin
)
