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

if (QT_FOUND)
	include_directories(
		src 
		${CMAKE_CURRENT_BINARY_DIR} 
		${QT_INCLUDE_DIR}
	)

	add_definitions( ${QT_DEFINITIONS} )

	# GEMPIL
	qt3_wrap_cpp(GEMPIL_QT_HEADERS_MOC ".moc.cpp"
		src/AppTimer.h
		src/GempisThread.h
		src/GlobalData.h
		src/JobLauncherController.h
		src/LocalCommandController.h
		src/LogFile.h
		src/ResourceFile.h
		src/ResourceLimits.h
		src/SubmitStatusController.h 
	)

	qt3_wrap_ui(GEMPIL_QT_FORMS ".hpp" ".cpp" ".moc.cpp"
		src/JobLauncherUI.ui 
		src/LocalCommandUI.ui 
		src/SubmitStatusUI.ui 
 	)

	add_executable(gempil
		src/AppTimer.C
		src/GempisThread.C
		src/GlobalData.C
		src/JobLauncherController.C
		src/LocalCommandController.C
		src/LogFile.C
		src/ResourceFile.C
		src/ResourceLimits.C
		src/SubmitStatusController.C 
		src/JobLauncherMain.C
		${GEMPIL_QT_HEADERS_MOC}
		${GEMPIL_QT_FORMS}	
	)

	target_link_libraries(gempil
		${QT_LIBRARIES}
	)

	# GEMPIR
	qt3_wrap_cpp(GEMPIR_QT_HEADERS_MOC ".moc.cpp"
		src/AppTimer.h
		src/GempisThread.h
		src/GlobalData.h
		src/LogFile.h
		src/ResourceManagerController.h
		src/ResourceFile.h
		src/ResourceEditorController.h
	)

	qt3_wrap_ui(GEMPIR_QT_FORMS ".hpp" ".cpp" ".moc.cpp"
		src/ResourceManagerUI.ui
		src/ResourceEditorUI.ui 
	)

	add_executable(gempir
		src/AppTimer.C
		src/GempisThread.C
		src/GlobalData.C
		src/LogFile.C
		src/ResourceManagerController.C
		src/ResourceFile.C
		src/ResourceEditorController.C
		src/ResourceManagerMain.C
		${GEMPIR_QT_HEADERS_MOC}
		${GEMPIR_QT_FORMS}
	)

	target_link_libraries(gempir
		${QT_LIBRARIES}
	)

	install(TARGETS gempil gempir
		RUNTIME DESTINATION bin
	)

endif(QT_FOUND)

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
