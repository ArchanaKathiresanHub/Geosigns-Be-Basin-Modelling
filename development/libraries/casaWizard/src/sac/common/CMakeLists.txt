file(GLOB CASAWIZARD_SAC_H_HEADERS
   view/*.h
   view/assets/*.h
   view/plots/*.h
   control/*.h
   control/assets/*.h)

file(GLOB CASAWIZARD_SAC_CPP_SOURCES
   view/*.cpp
   view/assets/*.cpp
   view/plots/*.cpp
   control/*.cpp
   control/assets/*.cpp)

file(GLOB CASAWIZARD_SAC_MODEL_H_HEADERS
   model/*.h
   model/extractor/*.h
   model/input/*.h
   model/output/*.h
   model/script/*.h )

file(GLOB CASAWIZARD_SAC_MODEL_CPP_SOURCES
   model/*.cpp
   model/extractor/*.cpp
   model/input/*.cpp
   model/output/*.cpp
   model/script/*.cpp )

include_directories( ${QTXLSX_INCLUDE_DIRS} )

create_bm_library( TARGET ${CASAWIZARD_SAC_LIBNAME}
   LIBRARIES Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Charts Qt5::Svg ${CASAWIZARD_COMMON_LIBNAME} utilities WorkLoadManagers
   ADD_HEADERS ${CASAWIZARD_SAC_H_HEADERS}
   ADD_SOURCES ${CASAWIZARD_SAC_CPP_SOURCES} )

create_bm_library( TARGET ${CASAWIZARD_SAC_MODEL_LIBNAME}
   LIBRARIES Qt5::Core ${CASAWIZARD_COMMON_MODEL_LIBNAME} WorkLoadManagers
   ADD_HEADERS ${CASAWIZARD_SAC_MODEL_H_HEADERS}
   ADD_SOURCES ${CASAWIZARD_SAC_MODEL_CPP_SOURCES} )

target_include_directories( ${CASAWIZARD_SAC_LIBNAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories( ${CASAWIZARD_SAC_MODEL_LIBNAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
