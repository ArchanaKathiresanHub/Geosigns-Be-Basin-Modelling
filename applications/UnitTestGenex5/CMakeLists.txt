add_executable(UnitTestGenex5 src/main.C)

bm_include_libraries(
	genex5_kernel
	CBMGenerics
	EosPack
)
target_link_libraries(UnitTestGenex5
	genex5_kernel
	CBMGenerics
	EosPack
)
