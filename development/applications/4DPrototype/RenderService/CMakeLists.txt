bm_include_libraries(
	DataAccess
	DataModel
	TableIO)

include_directories(
	$ENV{OIVHOME}/include
	../)

link_directories($ENV{OIVHOME}/$ENV{OIVARCH}/lib)

add_executable(RenderService
	BpaRenderAreaListener.cpp
	BpaServiceListener.cpp
	main.cpp
	RenderService.cpp
	SceneExaminer.cpp
	SceneInteractor.cpp)

target_link_libraries(RenderService
	Visualization
	DataAccess
	SerialDataAccess
	RemoteVizServices
	MeshVizDataMapping)

