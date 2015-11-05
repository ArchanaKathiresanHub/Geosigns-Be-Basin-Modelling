bm_include_libraries(
	DataAccess
	DataModel
	TableIO)

include_directories(
	$ENV{OIVHOME}/include)

add_library(Visualization
	CameraUtil.cpp
	Compass.cpp
	FlowLines.cpp
	Mesh.cpp
	OutlineBuilder.cpp
	Property.cpp
	ROICellFilter.cpp
	SceneGraphManager.cpp
	Traps.cpp)






