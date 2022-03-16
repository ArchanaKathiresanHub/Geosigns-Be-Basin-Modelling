 file(GLOB ctcWIZARD_H_HEADERS model/*.h view/*.h control/*.h )
 file(GLOB ctcWIZARD_CPP_SOURCES model/*.cpp view/*.cpp control/*.cpp )

include_directories(    ${QTXLSX_INCLUDE_DIRS} 
                        ${HDF5_INCLUDE_DIRS}
)

create_bm_library( TARGET ${CTCWIZARD_LIBNAME}
                   LIBRARIES Qt5::Core Qt5::Gui Qt5::Widgets ${Boost_LIBRARIES} QtUtils cmbapi
   CBMGenerics
   DataModel
   DataAccess
   SerialDataAccess
   Serial_Hdf5
   TableIO
   utilities
   ${HDF5_LIBRARIES}
                   ADD_HEADERS ${ctcWIZARD_H_HEADERS}
                   ADD_SOURCES ${ctcWIZARD_CPP_SOURCES} )

target_include_directories( ${CTCWIZARD_LIBNAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )
target_include_directories( ${CTCWIZARD_LIBNAME} PUBLIC ${PROJECT_SOURCE_DIR}/libraries/QT_Utils/src
    ${PROJECT_SOURCE_DIR}/libraries/WorkLoadManagers/src
)
