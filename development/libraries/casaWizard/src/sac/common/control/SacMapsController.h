//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QObject>
#include <memory>

namespace casaWizard
{

class ScriptRunController;

namespace sac
{

class SacMapsManager;
class SacScenario;
class SacMapsTab;
class SacVisualisationController;
class Generate3DScenarioScript;
class SacInfoGenerator;
class ActiveWellsController;

class SacMapsController : public QObject
{
   Q_OBJECT

public:
   explicit SacMapsController(ScriptRunController& scriptRunController, QObject* parent);
   virtual ~SacMapsController() = default;

private slots:
   void slotInterpolationTypeCurrentIndexChanged(int interpolationType);
   void slotPvalueChanged(int pIDW);
   void slotSmoothingTypeCurrentIndexChanged(int smoothingType);
   void slotSmoothingRadiusValueChanged(int smoothingRadius);
   void slotUpdateBirdView();
   void slotUpdateWellSelection();
   void slotWellClicked(const QString& wellName);
   void slotGenerateMaps();

   void slotExportOptimized();
   void slotExportOptimizedToZycor();
   void slotRunOptimized();

   void slotSelectAllWells() ;
   void slotDeselectAllWells();

signals:
   void signalRefreshChildWidgets();

protected:
   void init();
   virtual SacMapsManager& mapsManager() = 0;
   virtual SacMapsTab* mapsTab() = 0;
   virtual SacScenario& scenario() = 0;

   virtual SacVisualisationController& visualisationController() = 0;
   virtual std::unique_ptr<Generate3DScenarioScript> generate3DScenarioScript() = 0;
   virtual std::unique_ptr<SacInfoGenerator> infoGenerator() = 0;

   QVector<int> getSelectedWellIndices();
   ActiveWellsController* activeWellsController();
   ScriptRunController& scriptRunController();

   void validateWellsHaveOptimized();
   virtual void refreshGUI();

private:
   ScriptRunController& m_scriptRunController;
   ActiveWellsController* m_activeWellsController;
};

} // namespace sac

} // namespace casaWizard
