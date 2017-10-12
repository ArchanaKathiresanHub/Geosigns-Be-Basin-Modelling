//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "../src/ConfigFileParameterCtc.h"

#include <gtest/gtest.h>

// std library
#include <cmath>

using namespace CrustalThickness;

TEST( ConfigFileCtc, parameters ){
   ConfigFileParameterCtc parameters;

   //Load the configuration file and test values
   parameters.loadConfigurationFileCtc( "InterfaceData.cfg" );
   EXPECT_EQ( 3.45E-5, parameters.getCoeffThermExpansion()         );
   EXPECT_EQ( 4055,    parameters.getInitialSubsidenceMax()        );
   EXPECT_EQ( 2656,    parameters.getE0()                          );
   EXPECT_EQ( 42.9,    parameters.getTau()                         );
   EXPECT_EQ( 92500,   parameters.getModelTotalLithoThickness()    );
   EXPECT_EQ( 3205,    parameters.getBackstrippingMantleDensity()  );
   EXPECT_EQ( 3360,    parameters.getLithoMantleDensity()          );
   EXPECT_EQ( 1330,    parameters.getBaseLithosphericTemperature() );
   EXPECT_EQ( 35000,   parameters.getReferenceCrustThickness()     );
   EXPECT_EQ( 2875,    parameters.getReferenceCrustDensity()       );
   EXPECT_EQ( 1035,    parameters.getWaterDensity()                );
   EXPECT_EQ( 2.9881,  parameters.getA()                           );
   EXPECT_EQ( 1050,    parameters.getB()                           );
   EXPECT_EQ( 280,     parameters.getC()                           );
   EXPECT_EQ( -294000, parameters.getD()                           );
   EXPECT_EQ( 2830,    parameters.getE()                           );
   EXPECT_EQ( 2830,    parameters.getF()                           );
   EXPECT_EQ( 10000,   parameters.getDecayConstant()               );

   //Set value and test
   parameters.setDecayConstant( 300.156 );
   EXPECT_EQ( 3.45E-5, parameters.getCoeffThermExpansion()         );
   EXPECT_EQ( 4055,    parameters.getInitialSubsidenceMax()        );
   EXPECT_EQ( 2656,    parameters.getE0()                          );
   EXPECT_EQ( 42.9,    parameters.getTau()                         );
   EXPECT_EQ( 92500,   parameters.getModelTotalLithoThickness()    );
   EXPECT_EQ( 3205,    parameters.getBackstrippingMantleDensity()  );
   EXPECT_EQ( 3360,    parameters.getLithoMantleDensity()          );
   EXPECT_EQ( 1330,    parameters.getBaseLithosphericTemperature() );
   EXPECT_EQ( 35000,   parameters.getReferenceCrustThickness()     );
   EXPECT_EQ( 2875,    parameters.getReferenceCrustDensity()       );
   EXPECT_EQ( 1035,    parameters.getWaterDensity()                );
   EXPECT_EQ( 2.9881,  parameters.getA()                           );
   EXPECT_EQ( 1050,    parameters.getB()                           );
   EXPECT_EQ( 280,     parameters.getC()                           );
   EXPECT_EQ( -294000, parameters.getD()                           );
   EXPECT_EQ( 2830,    parameters.getE()                           );
   EXPECT_EQ( 2830,    parameters.getF()                           );
   EXPECT_EQ( 300.156, parameters.getDecayConstant()               );

   //Copy and test
   ConfigFileParameterCtc parametersCopy = parameters;
   parameters.setDecayConstant( 300.156 );
   EXPECT_EQ( 3.45E-5, parametersCopy.getCoeffThermExpansion()         );
   EXPECT_EQ( 4055,    parametersCopy.getInitialSubsidenceMax()        );
   EXPECT_EQ( 2656,    parametersCopy.getE0()                          );
   EXPECT_EQ( 42.9,    parametersCopy.getTau()                         );
   EXPECT_EQ( 92500,   parametersCopy.getModelTotalLithoThickness()    );
   EXPECT_EQ( 3205,    parametersCopy.getBackstrippingMantleDensity()  );
   EXPECT_EQ( 3360,    parametersCopy.getLithoMantleDensity()          );
   EXPECT_EQ( 1330,    parametersCopy.getBaseLithosphericTemperature() );
   EXPECT_EQ( 35000,   parametersCopy.getReferenceCrustThickness()     );
   EXPECT_EQ( 2875,    parametersCopy.getReferenceCrustDensity()       );
   EXPECT_EQ( 1035,    parametersCopy.getWaterDensity()                );
   EXPECT_EQ( 2.9881,  parametersCopy.getA()                           );
   EXPECT_EQ( 1050,    parametersCopy.getB()                           );
   EXPECT_EQ( 280,     parametersCopy.getC()                           );
   EXPECT_EQ( -294000, parametersCopy.getD()                           );
   EXPECT_EQ( 2830,    parametersCopy.getE()                           );
   EXPECT_EQ( 2830,    parametersCopy.getF()                           );
   EXPECT_EQ( 300.156, parametersCopy.getDecayConstant()               );

}