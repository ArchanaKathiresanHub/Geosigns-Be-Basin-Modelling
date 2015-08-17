bm_include_libraries(
	DataAccess
	DataModel
	TableIO)

include_directories(
	$ENV{OIVHOME}/include)

add_library(Visualization
	BpaMesh.cpp
	Extractor.cpp
	Mesh.cpp
	ROICellFilter.cpp
	SceneGraphManager.cpp
	SkinExtractor.cpp
	Property.cpp)




