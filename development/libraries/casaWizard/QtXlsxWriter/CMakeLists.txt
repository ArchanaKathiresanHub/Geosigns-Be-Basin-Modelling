set( CMAKE_AUTOMOC ON )
file(GLOB QTXLSX_HEADERS "*.h" )
file(GLOB QTXLSX_SOURCES "*.cpp" )

set( QTXLSX_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}" CACHE INTERNAL "")

# This include directory is explicitly required to find the <private/*> includes
include_directories( ${Qt5Gui_PRIVATE_INCLUDE_DIRS} )

add_definitions( -DXLSX_NO_LIB )

add_library( QtXlsx ${QTXLSX_HEADERS} ${QTXLSX_SOURCES} )
target_link_libraries( QtXlsx Qt5::Core Qt5::Gui )
