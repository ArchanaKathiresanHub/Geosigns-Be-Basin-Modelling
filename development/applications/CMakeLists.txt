
# Applications that can only be built on Unix at the moment.
if (UNIX)
	# Tests
	# add_subdirectory(EosPackDemo)
	# add_subdirectory(PreferredDirectionUnitTest)
	# add_subdirectory(TestDataAccess), Compilation error
	# add_subdirectory(TestDistributedDataAccess), Compilation error
	# add_subdirectory(UnitTestGenex5), Compilation error
	# add_subdirectory(UnitTestGenex6), Compilation error

	# Limited build applications
	if (BM_PARALLEL)
		add_subdirectory(fastcauldron)
		add_subdirectory(fastmig)
		add_subdirectory(fastgenex6)
		add_subdirectory(fasttouch7)
		add_subdirectory(fastctc)
	endif()
	if (BM_BUILD_SERIAL_APPS)
		add_subdirectory(cauldron2bhf)
		add_subdirectory(cauldron2eclipse)
		add_subdirectory(cauldron2voxet)
		add_subdirectory(gempis)
		add_subdirectory(fastreq)
		add_subdirectory(isosurface)
		add_subdirectory(track1d)
		add_subdirectory(xcauldron)
		add_subdirectory(datadriller)
		add_subdirectory(fastmiglogger)
		add_subdirectory(tracktraps)
		add_subdirectory(geomorph)
		add_subdirectory(deviatedwell)
		add_subdirectory(gempis_send)
		add_subdirectory(gempis_recv)
  		add_subdirectory(inversion_workbench)
  		add_subdirectory(eosptdiag)
	endif (BM_BUILD_SERIAL_APPS)


	# Complementary build applications
	if (BM_BUILD_COMPLEMENTARY_APPS)
		if (BM_BUILD_SERIAL_APPS)
			add_subdirectory(upgrade)
			add_subdirectory(setupView4D)
			add_subdirectory(trapper)
		endif(BM_BUILD_SERIAL_APPS)
	endif(BM_BUILD_COMPLEMENTARY_APPS)
endif(UNIX)

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
