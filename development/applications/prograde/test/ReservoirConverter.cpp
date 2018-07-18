//
// Copyright (C) 2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell .
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

//
#include <gtest/gtest.h>


#include "../src/ReservoirConverter.h"
#include "../src/ReservoirManager.h"

// Globle trap capacity value is the minimum of all the layer-wise values
TEST(ReservoirConverter, trapCapacity)
{
   Prograde::ReservoirConverter resConverter;
   double globalTrapCapacity = 1.0e10;
   double trapCapa = 500000.0;
   resConverter.trapCapacityLogic(trapCapa, globalTrapCapacity);
   trapCapa = 300000.0;
   resConverter.trapCapacityLogic(trapCapa, globalTrapCapacity);
   trapCapa = 400000.0;
   resConverter.trapCapacityLogic(trapCapa, globalTrapCapacity);

   EXPECT_NEAR(globalTrapCapacity, 300000.0, 1e-6);
}
//If BlockingPermeability is on and all layer-wise values are equal it will be taken as global value, else default (1e-9).
//This test checks BlockingPermeability is same in all layers  
TEST(ReservoirConverter, equalBlockingPermeability)
{
   Prograde::ReservoirConverter resConverter;
   double globalblockingPermeability = 1e-9;
   double blockingPermeability = 1e-8;
   int resId = 0;
   resConverter.blockingPermeabilityLogic(resId, blockingPermeability, globalblockingPermeability);
   blockingPermeability = 1e-8;
   resId = 1;
   resConverter.blockingPermeabilityLogic(resId, blockingPermeability, globalblockingPermeability);
   blockingPermeability = 1e-8;
   resId = 2;
   resConverter.blockingPermeabilityLogic(resId, blockingPermeability, globalblockingPermeability);

   EXPECT_NEAR(globalblockingPermeability, 1e-8, 1e-6);
}
//This test checks BlockingPermeability is not same in all layers, therefore, default value must be selected
TEST(ReservoirConverter, unequalBlockingPermeability)
{
   Prograde::ReservoirConverter resConverter;
   double globalBlockingPermeability = 1e-9;
   double blockingPermeability = 1e-8;
   int resId = 0;
   resConverter.blockingPermeabilityLogic(resId, blockingPermeability, globalBlockingPermeability);
   blockingPermeability = 1e-7;
   resId = 1;
   resConverter.blockingPermeabilityLogic(resId, blockingPermeability, globalBlockingPermeability);
   blockingPermeability = 1e-8;
   resId = 2;
   resConverter.blockingPermeabilityLogic(resId, blockingPermeability, globalBlockingPermeability);

   EXPECT_NEAR(globalBlockingPermeability, 1e-9, 1e-6);
}
//Bio-degradation is ON(1), if it is ON(1) for any reservoirs, else OFF(0)
//This test checks BioDegradInd is On(1) 
TEST(ReservoirConverter, bioDegradIndOn)
{
   Prograde::ReservoirConverter resConverter;
   int globalBioDegradInd = 0;
   int bioDegradInd = 0;
   resConverter.bioDegradIndLogic(bioDegradInd, globalBioDegradInd);
   bioDegradInd = 1;
   resConverter.bioDegradIndLogic(bioDegradInd, globalBioDegradInd);
   bioDegradInd = 0;
   resConverter.bioDegradIndLogic(bioDegradInd, globalBioDegradInd);

   EXPECT_EQ(globalBioDegradInd, 1);
}
//This test checks BioDegradInd is On(1), therefore for all reservoirs BioDegradIndis OFF(0)
TEST(ReservoirConverter, bioDegradIndOff)
{
   Prograde::ReservoirConverter resConverter;
   int globalBioDegradInd = 0;
   int bioDegradInd = 0;
   resConverter.bioDegradIndLogic(bioDegradInd, globalBioDegradInd);
   bioDegradInd =0;
   resConverter.bioDegradIndLogic(bioDegradInd, globalBioDegradInd);
   bioDegradInd = 0;
   resConverter.bioDegradIndLogic(bioDegradInd, globalBioDegradInd);

   EXPECT_EQ(globalBioDegradInd, 0);
}
//OilToGasCrackingInd is ON(1), if it is ON(1) for any reservoirs, else OFF(0)
//This test checks OilToGasCrackingInd is On(1)
TEST(ReservoirConverter, oilToGasCrackingIndOn)
{
   Prograde::ReservoirConverter resConverter;
   int globalOilToGasCrackingInd = 0;
   int oilToGasCrackingInd = 0;
   resConverter.oilToGasCrackingIndLogic(oilToGasCrackingInd, globalOilToGasCrackingInd);
   oilToGasCrackingInd = 0;
   resConverter.oilToGasCrackingIndLogic(oilToGasCrackingInd, globalOilToGasCrackingInd);
   oilToGasCrackingInd = 1;
   resConverter.oilToGasCrackingIndLogic(oilToGasCrackingInd, globalOilToGasCrackingInd);

   EXPECT_EQ(globalOilToGasCrackingInd, 1);
}
//This test checks OilToGasCrackingInd is On(1), therefore for all reservoirs OilToGasCrackingInd is OFF(0)
TEST(ReservoirConverter, oilToGasCrackingIndOff)
{
   Prograde::ReservoirConverter resConverter;
   int globalOilToGasCrackingInd = 0;
   int oilToGasCrackingInd = 0;
   resConverter.oilToGasCrackingIndLogic(oilToGasCrackingInd, globalOilToGasCrackingInd);
   oilToGasCrackingInd = 0;
   resConverter.oilToGasCrackingIndLogic(oilToGasCrackingInd, globalOilToGasCrackingInd);
   oilToGasCrackingInd = 0;
   resConverter.oilToGasCrackingIndLogic(oilToGasCrackingInd, globalOilToGasCrackingInd);

   EXPECT_EQ(globalOilToGasCrackingInd, 0);
}
//Block vertical migration will be ON(1), if it is ON(1) for all reservoirs, else OFF(0)
//This test checks BlockingInd is On(1)
TEST(ReservoirConverter, blockingIndOn)
{
   Prograde::ReservoirConverter resConverter;
   int globalBlockingInd = 1;
   int blockingInd = 1;
   resConverter.blockingIndLogic(blockingInd, globalBlockingInd);
   blockingInd = 1;
   resConverter.blockingIndLogic(blockingInd, globalBlockingInd);
   blockingInd = 1;
   resConverter.blockingIndLogic(blockingInd, globalBlockingInd);

   EXPECT_EQ(globalBlockingInd, 1);
}
//This test checks BlockingInd is OFF(0)
TEST(ReservoirConverter, blockingIndOff)
{
   Prograde::ReservoirConverter resConverter;
   int globalBlockingInd = 1;
   int blockingInd = 1;
   resConverter.blockingIndLogic(blockingInd, globalBlockingInd);
   blockingInd = 0;
   resConverter.blockingIndLogic(blockingInd, globalBlockingInd);
   blockingInd = 1;
   resConverter.blockingIndLogic(blockingInd, globalBlockingInd);

   EXPECT_EQ(globalBlockingInd, 0);
}

