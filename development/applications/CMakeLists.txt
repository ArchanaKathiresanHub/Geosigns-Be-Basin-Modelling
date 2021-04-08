#########################################################################
#                                                                       #
# Copyright (C) 2012-2015 Shell International Exploration & Production. #
# All rights reserved.                                                  #
#                                                                       #
# Developed under license for Shell by PDS BV.                          #
#                                                                       #
# Confidential and proprietary source code of Shell.                    #
# Do not distribute without written permission from Shell.              #
#                                                                       #
#########################################################################

set( BASE_FOLDER "Applications" )

include( ApplicationRunScript.cmake )

if (BM_BUILD_SERIAL_APPS)
    add_subdirectory(anonymizer)
    add_subdirectory(eosptdiag)
    add_subdirectory(EosPackDemo)
    add_subdirectory(track1d)
    add_subdirectory(datadriller)
    add_subdirectory(casa)
    add_subdirectory(prograde)
    add_subdirectory(projdiff)
    add_subdirectory(genex0d)
    add_subdirectory(modelPseudo1d)
	add_subdirectory(ctcWizard)
endif (BM_BUILD_SERIAL_APPS)

# Applications that can only be built on Unix at the moment.
if (UNIX)
    if (BM_PARALLEL)
        add_subdirectory(fastcauldron)
        add_subdirectory(fastmig)
        add_subdirectory(fastgenex6)
        add_subdirectory(fastctc)
        add_subdirectory(fastproperties)
        add_subdirectory(fastdepthconversion)
    endif()
    if (BM_BUILD_SERIAL_APPS)
        add_subdirectory(upcauldron)
        add_subdirectory(cauldron2eclipse)
        add_subdirectory(cauldron2voxet)
        add_subdirectory(gempis)
        add_subdirectory(isosurface)
        add_subdirectory(xcauldron)
        add_subdirectory(fastmiglogger)
        add_subdirectory(tracktraps)
        add_subdirectory(geomorph)
        add_subdirectory(deviatedwell)
        add_subdirectory(gempis_send)
        add_subdirectory(gempis_recv)
        add_subdirectory(hpc)
        add_subdirectory(cauldron-crop)
        add_subdirectory(UnitTestGenex6)
        add_subdirectory(UnitTestOTGC6)
        add_subdirectory(VisualizationIO_conversion)
        add_subdirectory(casaWizard)
        add_subdirectory(mapMorph)
        add_subdirectory(mapSmoother)
    endif (BM_BUILD_SERIAL_APPS)
# Windows
else ()
    add_subdirectory(cauldron2eclipse)
    add_subdirectory(fastcauldron)
    add_subdirectory(fastmig)
    add_subdirectory(fastctc)
    add_subdirectory(fastgenex6)
    add_subdirectory(fastproperties)
    add_subdirectory(hpc)
    add_subdirectory(tracktraps)
    add_subdirectory(VisualizationIO_conversion)
    add_subdirectory(fastdepthconversion)
endif ()
