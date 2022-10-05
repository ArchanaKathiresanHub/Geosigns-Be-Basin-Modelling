//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "McmcSettings.h"
#include "model/scenarioReader.h"
#include "model/scenarioWriter.h"

namespace casaWizard
{

namespace ua
{

McmcSettings::McmcSettings():
   m_standardDeviationFactor(1.6),
   m_nSamples(500)
{}

void McmcSettings::setNumSamples(int numSamples)
{
   m_nSamples = numSamples;
}

void McmcSettings::setStandardDeviationFactor(double factor)
{
   m_standardDeviationFactor = factor;
}

int McmcSettings::nSamples() const
{
   return m_nSamples;
}

double McmcSettings::standardDeviationFactor() const
{
   return m_standardDeviationFactor;
}

void McmcSettings::writeToFile(ScenarioWriter& writer) const
{
   writer.writeValue("standardDeviationFactor", m_standardDeviationFactor);
   writer.writeValue("nSamples",m_nSamples);
}

void McmcSettings::readFromFile(const ScenarioReader& reader)
{
   m_standardDeviationFactor = reader.readDouble("standardDeviationFactor");
   m_nSamples = reader.readInt("nSamples");
}

void McmcSettings::clear()
{
   //Set back to defaults:
   m_standardDeviationFactor = 1.6;
   m_nSamples = 500;
}

} // namespace ua
} // namespace casaWizard
