add_gtest( NAME StringSanitizer
   SOURCES TestStringSanitizer.cpp
   LIBRARIES Qt5::Core Qt5::Gui Qt5::Widgets QtUtils
   INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
   )
