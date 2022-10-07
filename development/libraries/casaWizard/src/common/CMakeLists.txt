file(GLOB CASAWIZARD_COMMON_MODEL_H_HEADERS model/*.h
                                            model/input/*.h
                                            model/output/*.h
                                            model/script/*.h
                                            model/functions/*.h )

file(GLOB CASAWIZARD_COMMON_MODEL_CPP_SOURCES model/*.cpp
                                              model/functions/*.cpp
                                              model/input/*.cpp
                                              model/input/*.hpp
                                              model/output/*.cpp
                                              model/script/*.cpp )

file(GLOB CASAWIZARD_COMMON_H_HEADERS view/*.h
                                      view/plot/*.h
                                      view/sharedComponents/*.h
                                      control/*.h
                                      model/functions/*.h )

file(GLOB CASAWIZARD_COMMON_CPP_SOURCES view/*.cpp
                                        view/plot/*.cpp
                                        view/sharedComponents/*.cpp
                                        view/sharedComponents/resources.qrc
                                        control/*.cpp
                                        model/functions/*.cpp )

include_directories( SYSTEM
  ${HDF5_INCLUDE_DIRS}
  ${CASAWIZARD_COMMON_INCLUDE_DIRS}
  ${QTXLSX_INCLUDE_DIRS}
)


create_bm_library( TARGET ${CASAWIZARD_COMMON_LIBNAME}
                   LIBRARIES Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Svg utilities QtUtils
                   ADD_SOURCES ${CASAWIZARD_COMMON_CPP_SOURCES}
                   ADD_HEADERS ${CASAWIZARD_COMMON_H_HEADERS} )

create_bm_library( TARGET ${CASAWIZARD_COMMON_MODEL_LIBNAME}
                  LIBRARIES Qt5::Core QtXlsx cmbapi utilities DataAccess SerialDataAccess TableIO DataModel DataExtraction QtUtils
                  ADD_SOURCES ${CASAWIZARD_COMMON_MODEL_CPP_SOURCES}
                  ADD_HEADERS ${CASAWIZARD_COMMON_MODEL_H_HEADERS} )


target_include_directories( ${CASAWIZARD_COMMON_LIBNAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories( ${CASAWIZARD_COMMON_MODEL_LIBNAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

