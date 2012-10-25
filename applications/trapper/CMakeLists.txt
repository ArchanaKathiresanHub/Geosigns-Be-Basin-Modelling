if (QT_FOUND)

qt3_wrap_ui(TRAPPER_QT_FORMS ".h" ".cpp" ".moc.cpp"
	src/axisRange.ui
	src/componentgraphoptionsframe.ui
	src/graphform.ui
	src/traptracking.ui
	src/usersettings.ui
)

qt3_wrap_images(TRAPPER_QT_IMAGES trapper
	images/print.png
	images/trapper_splash_screen.png
	images/zoomin.png
	images/zoomout.png
)

add_executable(trapper
	src/commandthread.C
	src/componentgraphform.C
	src/componentsreader.C
	src/componentsreaderpvt.C
	src/createuserrequest.C
	src/datareader.C
	src/directoryinfo.C
	src/environmentsettings.C
	src/eventnode.C
	src/eventtree.C
	src/filestatus.C
	src/graphfactory.C
	src/graphobjectslist.C
	src/interactivegraphform.C
	src/lineattributes.C
	src/linedata.C
	src/nodelist.C
	src/persistrapfileio.C
	src/persistrapfilestdio.C
	src/persistraps.C
	src/pipemessage.C
	src/propertiesreader.C
	src/readonlydatabase.C
	src/rightaxisgraphform.C
	src/sourcegraphform.C
	src/sourcereader.C
	src/trapchangeevent.C
	src/trapgrid.C
	src/traplist.C
	src/trapper_main.C
	src/trapsource.C
	src/traptrackingkernel.C
	src/volumereader.C
	${TRAPPER_QT_FORMS}
	${TRAPPER_QT_IMAGES}
)
	
bm_include_libraries(
	Serial_Hdf5
	TableIO
	CBMGenerics
	EosPack
)

include_directories(
	${HDF5_INCLUDE_DIRS}
	${QT_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	src
	${QWT_INCLUDE_DIRS}
)

target_link_libraries(trapper
	Serial_Hdf5
	TableIO
	CBMGenerics
	EosPack
	${HDF5_C_LIBRARIES}
	${QT_LIBRARIES}
	${QWT_LIBRARIES}
)

add_definitions(${QT_DEFINITIONS})

install(TARGETS trapper
	RUNTIME DESTINATION bin
	COMPONENT Complementary
)

endif(QT_FOUND)
