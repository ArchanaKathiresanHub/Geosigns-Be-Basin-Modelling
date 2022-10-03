add_casawizard_sac_gtest( NAME T2ZInfoGenerator
                          SOURCES testT2ZInfoGenerator.cpp )
add_casawizard_sac_gtest( NAME LithoMapsInfoGenerator
                          SOURCES testLithoMapsInfoGenerator.cpp )

configure_file( infoExpected.txt infoExpected.txt COPYONLY)
configure_file( infoT2ZOnOriginalExpected.txt infoT2ZOnOriginalExpected.txt COPYONLY)
configure_file( infoT2ZOnOptimizedExpected.txt infoT2ZOnOptimizedExpected.txt COPYONLY)
configure_file( infoLithoMapsExpected.txt infoLithoMapsExpected.txt COPYONLY)
configure_file( T2ZProject.project3d T2ZProject.project3d COPYONLY )
configure_file( LithoMapsProject.project3d LithoMapsProject.project3d COPYONLY )
