add_executable(UpgradeProject src/UpgradeProject.C)

bm_include_libraries(
	TableIO
)

target_link_libraries(UpgradeProject
	TableIO
)

install(TARGETS UpgradeProject
	RUNTIME DESTINATION bin
)

# Local Variables:

# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
