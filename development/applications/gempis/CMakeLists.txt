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



# GEMPIS
add_executable(pstree       src/pstree.c)
add_executable(parselog     src/ParseLog.C)
add_executable(getnextjobid 
	src/File.C 
	src/FileLock.C 
	src/JobId.C
	src/Signal.C
	src/GetNextJobid.C
)

install(TARGETS pstree parselog getnextjobid
	RUNTIME DESTINATION bin
)

# GEMPIS scripts
macro(add_script SCRIPT_SOURCE SCRIPT_NAME)
	string(REGEX REPLACE "([^/]*/)*([^.]*)\\.ksh" "\\2" script_name ${SCRIPT_SOURCE})
	file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${script_name} "#!/bin/bash\n")
	file(READ ${CMAKE_CURRENT_SOURCE_DIR}/${SCRIPT_SOURCE} contents)
	file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/${script_name} "${contents}")
	set(${SCRIPT_NAME} ${CMAKE_CURRENT_BINARY_DIR}/${script_name})
endmacro(add_script)

set(SCRIPTS
	src/cbm_perf.ksh
	src/cfg2html.ksh
	src/cfg2html_linux.ksh
	src/cfg2html_sun.ksh
	src/gempil_setup.ksh
	src/gempir_setup.ksh
	src/gempis_adm.ksh
	src/gempis_cfg.ksh
	src/gempis_mpiwrapper.ksh
	src/gempis_mon.ksh
	src/gempis_perf.ksh
	src/gempis_postapp.ksh
	src/gempis_postjob.ksh
	src/gempis_preapp.ksh
	src/gempis_resource.ksh
	src/gempis_stat.ksh
	src/gempis_submit.ksh
	src/gempis_utils.ksh
)

foreach(script ${SCRIPTS})
	add_script(${script} script_name)
	install( PROGRAMS ${script_name}
		DESTINATION bin
	)
endforeach(script)

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
