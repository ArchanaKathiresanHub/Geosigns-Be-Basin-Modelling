add_casawizard_sac_gtest( NAME OptimizedLithofractionScript      SOURCES testOptimizedLithofractionScript.cpp )
add_casawizard_sac_gtest( NAME DepthConversionScript            SOURCES testDepthConversionScript.cpp )
configure_file(Project.project3d Project.project3d COPYONLY)
file( COPY ${CMAKE_CURRENT_SOURCE_DIR}/T2Z_step2 DESTINATION ${CMAKE_CURRENT_BINARY_DIR} )

