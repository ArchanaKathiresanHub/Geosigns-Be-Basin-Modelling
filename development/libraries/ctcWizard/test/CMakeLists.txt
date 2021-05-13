#Add sub-directories   
#add_subdirectory( control )
#add_subdirectory( data )
#add_subdirectory( model )
#add_subdirectory( view )

set( LIB_NAME ${CTCWIZARD_LIBNAME})
## Unit Tests
configure_file(./data/Project.project3d ${CMAKE_CURRENT_BINARY_DIR}/Project.project3d COPYONLY)
configure_file(./data/Inputs.HDF ${CMAKE_CURRENT_BINARY_DIR}/Inputs.HDF COPYONLY)

add_gtest( NAME "CTCWizard::UnitTest"
	SOURCES ./model/scenarioTest.cpp
	#INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src
	LIBRARIES WorkLoadManagers Qt5::Core Qt5::Gui Qt5::Widgets ${Boost_LIBRARIES} ${LIB_NAME}
	FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)