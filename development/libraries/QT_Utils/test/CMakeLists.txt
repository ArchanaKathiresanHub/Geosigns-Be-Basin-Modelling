add_gtest( NAME Qt_Utils
   SOURCES TestQt_Utils.cpp
   LIBRARIES Qt5::Core Qt5::Gui Qt5::Widgets QtUtils
   INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
   )
