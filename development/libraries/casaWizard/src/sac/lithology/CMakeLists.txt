file(GLOB CASAWIZARD_SACLITHO_H_HEADERS
   view/*.h
   view/plots/*.h
   view/assets/*.h
   control/*.h )

file(GLOB CASAWIZARD_SACLITHO_CPP_SOURCES
   view/*.cpp
   view/plots/*.cpp
   view/assets/*.cpp
   control/*.cpp )

file(GLOB CASAWIZARD_SACLITHO_MODEL_H_HEADERS
   model/*.h
   model/extractor/*.h
   model/input/*.h
   model/output/*.h
   model/script/*.h )

file(GLOB CASAWIZARD_SACLITHO_MODEL_CPP_SOURCES
   model/*.cpp
   model/extractor/*.cpp
   model/input/*.cpp
   model/output/*.cpp
   model/script/*.cpp )

include_directories( ${QTXLSX_INCLUDE_DIRS} )

create_bm_library( TARGET ${CASAWIZARD_SACLITHO_LIBNAME}
   LIBRARIES Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Charts Qt5::Svg ${CASAWIZARD_COMMON_LIBNAME} ${CASAWIZARD_SAC_LIBNAME} utilities WorkLoadManagers
   ADD_SOURCES ${CASAWIZARD_SACLITHO_CPP_SOURCES}
   ADD_SOURCES ${CASAWIZARD_SAC_CPP_SOURCES})

create_bm_library( TARGET ${CASAWIZARD_SACLITHO_MODEL_LIBNAME}
   LIBRARIES Qt5::Core ${CASAWIZARD_COMMON_MODEL_LIBNAME} ${CASAWIZARD_SAC_MODEL_LIBNAME} WorkLoadManagers
   ADD_SOURCES ${CASAWIZARD_SACLITHO_MODEL_CPP_SOURCES}
   ADD_SOURCES ${CASAWIZARD_SAC_MODEL_CPP_SOURCES})

target_include_directories( ${CASAWIZARD_SACLITHO_LIBNAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories( ${CASAWIZARD_SACLITHO_MODEL_LIBNAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
