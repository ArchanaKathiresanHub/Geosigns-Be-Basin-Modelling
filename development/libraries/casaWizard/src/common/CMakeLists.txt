file(GLOB CASAWIZARD_COMMON_H_HEADERS model/*.h
                                      view/*.h
                                      view/plot/*.h
                                      control/*.h
                                      model/functions/*.h
                                      model/input/*.h
                                      model/output/*.h
                                      model/script/*.h )

file(GLOB CASAWIZARD_COMMON_CPP_SOURCES model/*.cpp
                                        view/*.cpp
                                        view/plot/*.cpp
                                        control/*.cpp
                                        model/functions/*.cpp
                                        model/input/*.cpp
                                        model/input/*.hpp
                                        model/output/*.cpp
                                        model/script/*.cpp )

include_directories( SYSTEM
  ${HDF5_INCLUDE_DIRS}
  ${CASAWIZARD_COMMON_INCLUDE_DIRS}
  ${QTXLSX_INCLUDE_DIRS}
)

create_bm_library( TARGET ${CASAWIZARD_COMMON_LIBNAME}
                   LIBRARIES Qt5::Core Qt5::Gui Qt5::Widgets QtXlsx cmbapi utilities DataAccess TableIO DataModel 
                   ADD_SOURCES ${CASAWIZARD_COMMON_CPP_SOURCES}
                   ADD_HEADERS ${CASAWIZARD_COMMON_H_HEADERS} )

target_include_directories( ${CASAWIZARD_COMMON_LIBNAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
