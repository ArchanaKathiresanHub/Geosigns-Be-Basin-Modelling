file(GLOB CASAWIZARD_SAC_H_HEADERS model/*.h
                               view/*.h
                               view/plot/*.h
                               control/*.h
                               model/extractor/*.h
                               model/input/*.h
                               model/output/*.h
                               model/script/*.h )

file(GLOB CASAWIZARD_SAC_CPP_SOURCES model/*.cpp
                                 view/*.cpp
                                 view/plot/*.cpp
                                 control/*.cpp
                                 model/extractor/*.cpp
                                 model/input/*.cpp
                                 model/output/*.cpp
                                 model/script/*.cpp )

include_directories( ${QTXLSX_INCLUDE_DIRS} )

create_bm_library( TARGET ${CASAWIZARD_SAC_LIBNAME}
                   LIBRARIES Qt5::Core Qt5::Gui Qt5::Widgets ${CASAWIZARD_COMMON_LIBNAME}
                   ADD_HEADERS ${CASAWIZARD_SAC_H_HEADERS}
                   ADD_SOURCES ${CASAWIZARD_SAC_CPP_SOURCES} )
target_include_directories( ${CASAWIZARD_SAC_LIBNAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
