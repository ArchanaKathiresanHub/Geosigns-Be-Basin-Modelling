add_executable(EosPackDemo src/main.C)

bm_include_libraries(
	EosPack
	CBMGenerics
)

target_link_libraries( EosPackDemo
	EosPack
	CBMGenerics
)

install( TARGETS EosPackDemo
   DESTINATION	bin
)