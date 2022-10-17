file(GLOB CASAWIZARD_UA_H_HEADERS view/*.h
                                  view/plot/*.h
                                  control/*.h)

file(GLOB CASAWIZARD_UA_CPP_SOURCES view/*.cpp
                                    view/plot/*.cpp
                                    control/*.cpp )

file(GLOB CASAWIZARD_UA_MODEL_H_HEADERS model/*.h
                                  model/influentialParameter/*.h
                                  model/input/*.h
                                  model/output/*.h
                                  model/script/*.h )

file(GLOB CASAWIZARD_UA_MODEL_CPP_SOURCES model/*.cpp
                                    model/influentialParameter/*.cpp
                                    model/input/*.cpp
                                    model/output/*.cpp
                                    model/script/*.cpp )

include_directories( ${QTXLSX_INCLUDE_DIRS} )

create_bm_library( TARGET ${CASAWIZARD_UA_LIBNAME}
                   LIBRARIES Qt5::Core Qt5::Gui Qt5::Widgets ${CASAWIZARD_COMMON_LIBNAME}
                   ADD_SOURCES ${CASAWIZARD_UA_CPP_SOURCES}
                   ADD_HEADERS ${CASAWIZARD_UA_H_HEADER} )

create_bm_library( TARGET ${CASAWIZARD_UA_MODEL_LIBNAME}
                   LIBRARIES ${CASAWIZARD_COMMON_MODEL_LIBNAME} casaCmdInterface
                   ADD_SOURCES ${CASAWIZARD_UA_MODEL_CPP_SOURCES}
                   ADD_HEADERS ${CASAWIZARD_UA_MODEL_H_HEADER} )

target_include_directories( ${CASAWIZARD_UA_LIBNAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories( ${CASAWIZARD_UA_MODEL_LIBNAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
