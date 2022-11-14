//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "TCHPManager.h"

#include "model/logger.h"
#include "model/scenarioReader.h"
#include "model/scenarioWriter.h"

static const int version = 0;

namespace casaWizard
{

namespace sac
{

namespace thermal
{

TCHPManager::TCHPManager() :
   m_minHeatProduction{0},
   m_maxHeatProduction{5},
   m_optimizedTCHPs{}
{
}

double TCHPManager::minTCHP() const
{
   return m_minHeatProduction;
}

double TCHPManager::maxTCHP() const
{
   return m_maxHeatProduction;
}

void TCHPManager::setMinTCHP(const double value)
{
   m_minHeatProduction = value;
}

void TCHPManager::setMaxTCHP(const double value)
{
   m_maxHeatProduction = value;
}

const QVector<OptimizedTCHP>& TCHPManager::optimizedTCHPs() const
{
   return m_optimizedTCHPs;
}

OptimizedTCHP TCHPManager::optimizedInWell(const int wellIndex) const
{
   OptimizedTCHP optimizedInWell;
   for (const OptimizedTCHP& tchp : m_optimizedTCHPs)
   {
      if (tchp.wellId() == wellIndex)
      {
         optimizedInWell = tchp;
         break;
      }
   }
   return optimizedInWell;
}

void TCHPManager::addOptimizedTCHP(const OptimizedTCHP& optimal)
{
   m_optimizedTCHPs.push_back(optimal);
}

void TCHPManager::clearOptimizedTCHPs()
{
   m_optimizedTCHPs.clear();
}

void TCHPManager::writeToFile(ScenarioWriter& writer) const
{
   writer.writeValue("TCHPManagerVersion", version);
   writer.writeValue("minimalProduction", m_minHeatProduction);
   writer.writeValue("maximumProduction", m_maxHeatProduction);
   writer.writeValue("optimizedTopCrustHeatProduction", m_optimizedTCHPs);
}

void TCHPManager::readFromFile(const ScenarioReader& reader)
{
   m_minHeatProduction = reader.readDouble("minimalProduction");
   m_maxHeatProduction = reader.readDouble("maximumProduction");
   m_optimizedTCHPs = reader.readVector<OptimizedTCHP>("optimizedTopCrustHeatProduction");
}

void TCHPManager::clear()
{
   m_optimizedTCHPs.clear();
}

} // namespace thermal

} // namespace sac

} // namespace casaWizard
