//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Data class for the SAC Thermal Wizard
#pragma once

#include "model/SacScenario.h"
#include "model/TCHPManager.h"
#include "model/ThermalMapManager.h"

namespace casaWizard
{

class VectorVectorMap;
class CMBMapReader;

namespace sac
{

namespace thermal
{

class ThermalScenario : public SacScenario
{
public:
   ThermalScenario(ProjectReader* projectReader);

   virtual ThermalMapManager& mapsManager() final;
   virtual const ThermalMapManager& mapsManager() const final;

   TCHPManager& TCHPmanager();
   const TCHPManager& TCHPmanager() const;

   void writeToFile(ScenarioWriter& writer) const override;
   void readFromFile(const ScenarioReader& reader) override;
   void clear() override;

   void getVisualisationData(QVector<OptimizedTCHP>& optimizedTCHPs,
                             QVector<const Well*>& activeAndIncludedWells) const;
   bool openThermalMap(CMBMapReader& mapReader, bool& optimized);
   bool getThermalMap(VectorVectorMap& ThermalMaps);
   double getTCHPOfClosestWell(const double xInKm, const double yInKm, int& closestWellID) const;

private:
   TCHPManager m_TCHPManager;
   ThermalMapManager m_mapManager;
};

} // namespace thermal

} // namespace sac

} // namespace casaWizard
