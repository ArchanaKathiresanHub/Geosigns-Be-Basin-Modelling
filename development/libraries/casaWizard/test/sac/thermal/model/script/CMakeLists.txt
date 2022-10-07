#add_casawizard_sac_gtest( NAME OptimizedHCPScript      SOURCES testOptimizedHCPScript.cpp )

#configure_file(Project.project3d Project.project3d COPYONLY)

add_casawizard_sac_gtest( NAME SACThermalScript   SOURCES testSACThermalScript.cpp )
configure_file(SACThermalScriptExpected.casa SACThermalScriptExpected.casa COPYONLY)

