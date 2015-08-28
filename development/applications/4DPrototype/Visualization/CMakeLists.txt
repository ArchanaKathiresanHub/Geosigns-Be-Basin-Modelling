bm_include_libraries(
	DataAccess
	DataModel
	TableIO)

include_directories(
	$ENV{OIVHOME}/include)

add_library(Visualization
	Mesh.cpp
	ROICellFilter.cpp
	SceneGraphManager.cpp
	Property.cpp)




