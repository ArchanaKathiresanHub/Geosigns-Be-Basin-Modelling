#########################################################################
#                                                                       #
# Copyright (C) 2012-2016 Shell International Exploration & Production. #
# All rights reserved.                                                  #
#                                                                       #
# Developed under license for Shell by PDS BV.                          #
#                                                                       #
# Confidential and proprietary source code of Shell.                    #
# Do not distribute without written permission from Shell.              #
#                                                                       #
#########################################################################

# 1. This list contains all header files
FILE( GLOB all_headers src/*.h )
source_group(include FILES ${all_headers})

# 2. This list contains all implementation files except main.C
FILE( GLOB all_srcs src/*.cpp)
source_group(source FILES ${all_srcs})

# 3. Anonymizer application
add_executable( anonymizer
   ${all_srcs}
   ${all_headers}
)

# 4. Dependencies
if (UNIX)
   add_dependencies(anonymizer TableIO Boost)
else (WIN32)
   add_dependencies(anonymizer TableIO)
endif (UNIX)

include_directories( src )

include_directories( SYSTEM
   ${HDF5_INCLUDE_DIRS}
   ${PUGIXML_INCLUDE_DIR}
)

bm_include_libraries(
   TableIO
   FileSystem
   utilities
)

target_link_libraries(anonymizer
   TableIO
   FileSystem
   utilities
   PugiXMLlib
   ${HDF5_LIBRARIES}
   ${Boost_LIBRARIES}
)

install(TARGETS anonymizer
   DESTINATION bin
)

if( UNIX )
   # Copying comparison script to binary folder and setting permissions
   configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/compare.sh.in
                   ${CMAKE_CURRENT_SOURCE_DIR}/compare.sh
                   COPYONLY )
   file( COPY ${CMAKE_CURRENT_SOURCE_DIR}/compare.sh
         DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
         FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ )
   file( REMOVE ${CMAKE_CURRENT_SOURCE_DIR}/compare.sh )
endif()

# Copy files for unit test
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/test_cbm00)
# Original project file to be anonymized
configure_file( 
      ${CMAKE_CURRENT_SOURCE_DIR}/test/cbm00/Wyllie.project3d
      ${CMAKE_CURRENT_BINARY_DIR}/test_cbm00
      COPYONLY
)
# Original map HDF file to be anonymized
configure_file( 
      ${CMAKE_CURRENT_SOURCE_DIR}/test/cbm00/Inputs.HDF
      ${CMAKE_CURRENT_BINARY_DIR}/test_cbm00
      COPYONLY
)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/test_cbm00/anonymized2compare)
# Anonymized project file to be used as comparison reference
configure_file( 
      ${CMAKE_CURRENT_SOURCE_DIR}/test/cbm00/anonymized2compare/Project.anonymized.project3d
      ${CMAKE_CURRENT_BINARY_DIR}/test_cbm00/anonymized2compare
      COPYONLY
)
# Anonymized map HDF file to be used as comparison reference
configure_file( 
      ${CMAKE_CURRENT_SOURCE_DIR}/test/cbm00/anonymized2compare/MapFile000.anonymized.HDF
      ${CMAKE_CURRENT_BINARY_DIR}/test_cbm00/anonymized2compare
      COPYONLY
)
# File containing names mapping (original-to-anonymized) to be used as comparison reference
configure_file( 
      ${CMAKE_CURRENT_SOURCE_DIR}/test/cbm00/anonymized2compare/Wyllie.namesMapping.txt
      ${CMAKE_CURRENT_BINARY_DIR}/test_cbm00/anonymized2compare
      COPYONLY
)

# Unit test
add_gtest( NAME "Anonymizer"
           SOURCES src/anonymizer.cpp test/anonymizer.cpp
           LIBRARIES TableIO FileSystem utilities PugiXMLlib ${HDF5_LIBRARIES} ${Boost_LIBRARIES}
         )

# Doxygen
generate_dox( anonymizer.cfg )

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
