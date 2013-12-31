#########################################################################
#                                                                       #
# Copyright (C) 2012-2013 Shell International Exploration & Production. #
# All rights reserved.                                                  #
#                                                                       #
# Developed under license for Shell by PDS BV.                          #
#                                                                       #
# Confidential and proprietary source code of Shell.                    #
# Do not distribute without written permission from Shell.              #
#                                                                       #
#########################################################################

if (BM_BUILD_SERIAL_APPS)
	add_subdirectory(eosptdiag)
	add_subdirectory(EosPackDemo)
endif (BM_BUILD_SERIAL_APPS)

# Applications that can only be built on Unix at the moment.
if (UNIX)
	# Tests
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
		add_subdirectory(upcauldron)
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
		add_subdirectory(hpc)
		add_subdirectory(cauldron-crop)
		add_subdirectory(TsLibDemo)
		add_subdirectory(UnitTestGenex6)
		add_subdirectory(UnitTestOTGC6)
	endif (BM_BUILD_SERIAL_APPS)

endif(UNIX)

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
