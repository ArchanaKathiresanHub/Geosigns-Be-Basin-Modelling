//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "ThermalScenario.h"

#include "model/input/cmbMapReader.h"
#include "model/input/projectReader.h"
#include "model/scenarioReader.h"
#include "model/scenarioWriter.h"

#include "ConstantsMathematics.h"
#include "ConstantsNumerical.h"

#include "model/logger.h"

#include <QDir>

static const int s_ThermalScenarioVersion = 0;

namespace casaWizard
{

namespace sac
{

namespace thermal
{

ThermalScenario::ThermalScenario(ProjectReader* projectReader) :
   SacScenario{projectReader},
   m_TCHPManager{}
{
}

ThermalMapManager& ThermalScenario::mapsManager()
{
   return m_mapManager;
}

const ThermalMapManager& ThermalScenario::mapsManager() const
{
   return m_mapManager;
}

TCHPManager& ThermalScenario::TCHPmanager()
{
   return m_TCHPManager;
}

const TCHPManager& ThermalScenario::TCHPmanager() const
{
   return m_TCHPManager;
}

void ThermalScenario::writeToFile(ScenarioWriter& writer) const
{
   SacScenario::writeToFile(writer);
   writer.writeValue("ThermalScenarioVersion", s_ThermalScenarioVersion);

   m_TCHPManager.writeToFile(writer);
}

void ThermalScenario::readFromFile(const ScenarioReader& reader)
{
   SacScenario::readFromFile(reader);
   m_TCHPManager.readFromFile(reader);
}

void ThermalScenario::clear()
{
   SacScenario::clear();
   m_TCHPManager.clear();
}

void ThermalScenario::getVisualisationData(QVector<OptimizedTCHP> &optimizedTCHPs,
                                           QVector<const Well *> &activeAndIncludedWells) const
{
   optimizedTCHPs = m_TCHPManager.optimizedTCHPs();
   activeAndIncludedWells = calibrationTargetManager().activeAndIncludedWells();
}

bool ThermalScenario::openThermalMap(CMBMapReader& mapReader, bool& optimized)
{
   if (project3dFilename() == "")
   {
      return false;
   }

   if ( !QDir(optimizedProjectDirectory()).exists() )
   {
      optimized = false;
      mapReader.load(project3dPath().toStdString());
   }
   else
   {
      optimized = true;
      mapReader.load((optimizedProjectDirectory() + project3dFilename()).toStdString());
   }

   return mapReader.mapExists( mapReader.getTopCrustHeatProductionMapName() );
}

bool ThermalScenario::getThermalMap(VectorVectorMap& thermalMap)
{
   CMBMapReader mapReader;
   bool optimized = false;
   if( openThermalMap(mapReader, optimized) )
   {
      thermalMap = mapReader.getTopCrustHeatProductionMap();
   }

   thermalMap.setUndefinedValuesBasedOnReferenceMap( mapReader.getMapData(projectReader().getDepthGridName(0).toStdString()) );

   return optimized;
}

double ThermalScenario::getTCHPOfClosestWell(const double xInKm, const double yInKm, int& closestWellID) const
{
   double TCHPAtPoint = Utilities::Numerical::CauldronNoDataValue;
   double xMin = 0; double xMax = 1; double yMin = 0; double yMax = 1;
   projectReader().domainRange(xMin, xMax, yMin, yMax);
   double smallestDistance2 = ((xMax - xMin) / 30 )* ((xMax - xMin) / 30 );

   int closestTCHPIndex = -1;
   int currentTCHPIndex = 0;

   const QVector<OptimizedTCHP>& TCHPs = TCHPmanager().optimizedTCHPs();
   for (const auto& optimizedTCHP : TCHPs)
   {
      const int wellId = optimizedTCHP.wellId();
      const auto& currentWell = calibrationTargetManager().well(wellId);
      if (!currentWell.isActive()){
         continue;
      }
      const double distance2 = (currentWell.x() - xInKm * Utilities::Maths::KilometerToMeter) * (currentWell.x() - xInKm * Utilities::Maths::KilometerToMeter) +
                               (currentWell.y() - yInKm * Utilities::Maths::KilometerToMeter) * (currentWell.y() - yInKm * Utilities::Maths::KilometerToMeter);
      const bool isCloser = distance2 < smallestDistance2;
      if (isCloser)
      {
         smallestDistance2 = distance2;
         closestTCHPIndex = currentTCHPIndex;
         closestWellID = wellId;
      }
      currentTCHPIndex++;
   }

   if (closestTCHPIndex!= -1)
   {
      TCHPAtPoint = TCHPs[closestTCHPIndex].optimizedHeatProduction();
   }

   return TCHPAtPoint;
}

} // namespace thermal

} // namespace sac

} // namespace casaWizard
