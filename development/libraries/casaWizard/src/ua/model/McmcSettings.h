//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "model/writable.h"

namespace casaWizard
{

namespace ua
{

class McmcSettings : public Writable
{
public:
   McmcSettings();

   void setNumSamples(int numSamples);
   void setStandardDeviationFactor(double factor);

   int nSamples() const;
   double standardDeviationFactor() const;

   void writeToFile(ScenarioWriter& writer) const;
   void readFromFile(const ScenarioReader& reader);
   void clear();

private:
   double m_standardDeviationFactor;
   int m_nSamples;
};

} // namespace ua
} // namespace casaWizard
