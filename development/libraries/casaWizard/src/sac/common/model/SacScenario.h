//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Data class for the SAC Wizard
#pragma once

#include "model/casaScenario.h"
#include "model/wellTrajectoryManager.h"

namespace casaWizard
{

class MapReader;

namespace sac
{

class SacMapsManager;

class SacScenario : public CasaScenario
{
public:
   SacScenario(ProjectReader* projectReader);

   void writeToFile(ScenarioWriter& writer) const override;
   void readFromFile(const ScenarioReader& reader) override;
   void clear() override;
   QString iterationDirName() const override;

   virtual SacMapsManager& mapsManager() = 0;
   virtual const SacMapsManager& mapsManager() const = 0;

   void exportOptimizedMapsToZycor(const QString& targetPath);

   void wellPrepToSAC();
   bool hasOptimizedSuccessfully(const int caseIndex);
   void updateWellsForProject3D();

   void setFitRangeToData(const bool fitRangeToData);
   void setShowSurfaceLines(const bool showSurfaceLines);
   void setActivePlots(const QVector<bool>& activePlots);

   QString stateFileNameSAC() const;
   QString calibrationDirectory() const;
   QString optimizedProjectDirectory() const;
   bool fitRangeToData() const;
   bool showSurfaceLines() const;
   WellTrajectoryManager& wellTrajectoryManager();
   const WellTrajectoryManager& wellTrajectoryManager() const;
   QVector<bool> activePlots() const;
   QVector<int> getIncludedWellIndicesFromSelectedWells(const QVector<int>& selectedWellIndices);

   void setCalibrationTargetsBasedOnObjectiveFunctions();
protected:
   bool openMaps(MapReader& mapReader, const QString& mapName) const;

private:
   QString m_stateFileNameSAC;
   QString m_calibrationDirectory;

   WellTrajectoryManager m_wellTrajectoryManager;

   QVector<bool> m_activePlots;
   bool m_fitRangeToData;
   bool m_showSurfaceLines;
};

} // namespace sac

} // namespace casaWizard
