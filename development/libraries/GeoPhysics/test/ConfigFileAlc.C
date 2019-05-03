//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "../src/ConfigFileParameterAlc.h"

#include <gtest/gtest.h>

//std
#include <fstream>

// FileSystem
#include "FilePath.h"

using namespace GeoPhysics;

TEST(ConfigFileAlc, parameters){


   ConfigFileParameterAlc parameters;

   //Load the configuration file and test values
   std::ifstream  configurationFile;
   char * AlcConfigurationFile = getenv( "CTCDIR" );
   ibs::Path fp( AlcConfigurationFile );
   fp << "InterfaceData.cfg";
   std::string fullPath = fp.path();
   configurationFile.open( fullPath );
   parameters.loadConfigurationFileAlc( configurationFile );
   configurationFile.close();
   EXPECT_EQ( 2875     , parameters.m_csRho   );
   EXPECT_EQ( 2875     , parameters.m_clRho   );
   EXPECT_EQ( 2887     , parameters.m_bRho    );
   EXPECT_EQ( 3360     , parameters.m_mRho    );
   EXPECT_EQ( 0.00004  , parameters.m_csA     );
   EXPECT_EQ( 0        , parameters.m_csB     );
   EXPECT_EQ( 0.00004  , parameters.m_clA     );
   EXPECT_EQ( 0        , parameters.m_clB     );
   EXPECT_EQ( 0.00004  , parameters.m_bA      );
   EXPECT_EQ( 0        , parameters.m_bB      );
   EXPECT_EQ( 0.0000345, parameters.m_mA      );
   EXPECT_EQ( 0        , parameters.m_mB      );
   EXPECT_EQ( 1000     , parameters.m_bT      );
   EXPECT_EQ( 0.63     , parameters.m_bHeat   );

   //Copy and test
   ConfigFileParameterAlc parametersCopy = parameters;
   EXPECT_EQ( 2875     , parametersCopy.m_csRho   );
   EXPECT_EQ( 2875     , parametersCopy.m_clRho   );
   EXPECT_EQ( 2887     , parametersCopy.m_bRho    );
   EXPECT_EQ( 3360     , parametersCopy.m_mRho    );
   EXPECT_EQ( 0.00004  , parametersCopy.m_csA     );
   EXPECT_EQ( 0        , parametersCopy.m_csB     );
   EXPECT_EQ( 0.00004  , parametersCopy.m_clA     );
   EXPECT_EQ( 0        , parametersCopy.m_clB     );
   EXPECT_EQ( 0.00004  , parametersCopy.m_bA      );
   EXPECT_EQ( 0        , parametersCopy.m_bB      );
   EXPECT_EQ( 0.0000345, parametersCopy.m_mA      );
   EXPECT_EQ( 0        , parametersCopy.m_mB      );
   EXPECT_EQ( 1000     , parametersCopy.m_bT      );
   EXPECT_EQ( 0.63     , parametersCopy.m_bHeat   );
}
