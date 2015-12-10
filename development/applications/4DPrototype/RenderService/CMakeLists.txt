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
	CommandHandler.cpp
	main.cpp
	jsonxx.cpp
	RenderService.cpp
	SceneExaminer.cpp
	SceneInteractor.cpp)

target_link_libraries(RenderService
	Visualization
	DataAccess
	VisualizationIO
	SerialDataAccess
	RemoteVizServices
	MeshVizDataMapping
	MeshVizExtractor
	MeshViz
	Inventor
	InventorGL
	InventorBase
	dl)

