#include <gtest/gtest.h>
#include "model/McmcSettings.h"

TEST( McmcSettingsTest, testClear )
{
   using namespace casaWizard::ua;
   McmcSettings mcmcSettings;
   mcmcSettings.setNumSamples(600);
   mcmcSettings.setStandardDeviationFactor(2.0);
   mcmcSettings.clear();
   EXPECT_EQ(500, mcmcSettings.nSamples());
   EXPECT_DOUBLE_EQ(1.6, mcmcSettings.standardDeviationFactor());
}
