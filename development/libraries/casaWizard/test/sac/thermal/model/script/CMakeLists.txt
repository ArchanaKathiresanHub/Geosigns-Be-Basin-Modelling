add_casawizard_sac_gtest( NAME ThermalScript       SOURCES testThermalScript.cpp )
configure_file(ThermalScriptExpected.casa ThermalScriptExpected.casa COPYONLY)

add_casawizard_sac_gtest( NAME OptimizedTCHPScript    SOURCES testOptimizedTCHPScript.cpp )
#configure_file(Project.project3d Project.project3d COPYONLY)
