//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QObject>
#include <QVector>
#include <memory>

class QString;
class QTableWidgetItem;

namespace casaWizard
{

class CalibrationTargetController;
class ScriptRunController;
class CasaScenario;

namespace sac
{

class ObjectiveFunctionControllerSAC;
class DataExtractionController;
class SacInputTab;
class SacScenario;
class SACScript;

class SacInputController : public QObject
{
   Q_OBJECT

public:
   SacInputController(SacInputTab* inputTab,
                      SacScenario& casaScenario,
                      ScriptRunController& scriptRunController,
                      QObject* parent);

signals:
   void signalRefreshChildWidgets();

protected slots:
   virtual void slotUpdateTabGUI(int tabID) = 0;
   virtual void slotPushButtonSelectProject3dClicked() = 0;

   void slotRunOriginal1D();
   void slotRunOriginal3D();
   void slotPushButton1DOptimalizationClicked();

   virtual SacScenario& scenario() = 0;
   virtual SacScenario& scenario() const = 0;

protected:
   virtual std::unique_ptr<SACScript> optimizationScript(const QString& baseDirectory, bool doOptimization = true) = 0;
   virtual void readOptimizedResults() = 0;
   virtual void refreshGUI();

   bool selectWorkspace();
   void renameCaseFolders(const QString& pathName);

   bool prepareOptimizationRun() const;
   bool prepareRun1D() const;

   virtual SacInputTab* inputTab() = 0;
   virtual const SacInputTab* inputTab() const = 0;

   ScriptRunController& scriptRunController();
   CalibrationTargetController* calibrationTargetController();
   DataExtractionController* dataExtractionController();

private slots:
   void slotExtractData();
   void slotComboBoxClusterCurrentTextChanged(QString clusterName);
   void slotComboBoxApplicationChanged(QString application);

private:
   ScriptRunController& m_scriptRunController;
   CalibrationTargetController* m_calibrationTargetController;
   ObjectiveFunctionControllerSAC* m_objectiveFunctionController;
   DataExtractionController* m_dataExtractionController;
};

} // namespace sac

} // namespace casaWizard
