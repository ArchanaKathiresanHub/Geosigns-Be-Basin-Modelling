bm_include_libraries(
	DataAccess
	DataModel
	TableIO
	VisualizationIO)

include_directories(
	$ENV{OIVHOME}/include
	${HDF5_INCLUDE_DIRS})

add_library(Visualization
	CameraUtil.cpp
	Compass.cpp
	FlowLines.cpp
	Mesh.cpp
	OutlineBuilder.cpp
	Property.cpp
	ROICellFilter.cpp
	SceneGraphManager.cpp
	Project.cpp
	DataAccessProject.cpp
	VisualizationIOProject.cpp
	FluidContacts.cpp
	Traps.cpp)






