//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Data class for the SAC Wizard
#pragma once

#include "model/casaScenario.h"
#include "model/MapsManager.h"
#include "model/wellTrajectoryManager.h"

namespace casaWizard
{

namespace sac
{

class SacScenario : public CasaScenario
{
public:
  SacScenario(ProjectReader* projectReader);

  QString stateFileNameSAC() const;
  QString calibrationDirectory() const;
  QString optimizedProjectDirectory() const;

  bool fitRangeToData() const;
  void setFitRangeToData(const bool fitRangeToData);

  WellTrajectoryManager& wellTrajectoryManager();
  const WellTrajectoryManager& wellTrajectoryManager() const;

  MapsManager& mapsManager();
  const MapsManager& mapsManager() const;

  void writeToFile(ScenarioWriter& writer) const override;
  void readFromFile(const ScenarioReader& reader) override;
  void clear() override;
  QString iterationDirName() const override;

  QVector<bool> activePlots() const;
  void setActivePlots(const QVector<bool>& activePlots);
  void wellPrepToSAC();
  QVector<int> getIncludedWellIndicesFromSelectedWells(const QVector<int>& selectedWellIndices);
  bool hasOptimizedSuccessfully(const int caseIndex);
  void updateWellsForProject3D();

protected:
  bool openMaps(MapReader& mapReader, const int layerID) const;

private:
  QString m_stateFileNameSAC;
  QString m_calibrationDirectory;
  MapsManager m_mapsManager;
  WellTrajectoryManager m_wellTrajectoryManager;

  QVector<bool> m_activePlots;
  bool m_fitRangeToData;
};

} // namespace sac

} // namespace casaWizard
