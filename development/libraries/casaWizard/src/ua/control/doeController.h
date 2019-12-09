// Controller of the DoE tab of the thermal wizard
#pragma once

#include <QObject>
#include <QVector>

class QString;
class QTableWidgetItem;

namespace casaWizard
{

class ScriptRunController;

namespace ua
{

class UAScenario;
class DoeTab;
class InfluentialParameterController;
class ManualDesignPointController;

class DoEcontroller : public QObject
{
  Q_OBJECT

public:
  DoEcontroller(DoeTab* doeTab,
                UAScenario& casaScenario,
                ScriptRunController& scriptRunController,
                QObject* parent);

private slots:
  void slotPushSelectProject3dClicked();
  void slotPushButtonDoErunCasaClicked();
  void slotPushButtonRunAddedCasesClicked();

  void slotComboBoxApplicationCurrentTextChanged(const QString& applicationName);
  void slotComboBoxClusterCurrentTextChanged(const QString& clusterName);
  void slotLineEditProject3dTextChanged(const QString& project3dPath);
  void slotSpinBoxCpusValueChanged(int cpus);
  void slotManualDesignPointsChanged();
  void slotDoeSelectionItemChanged(QTableWidgetItem* item);

  void slotUpdateDoeOptionTable();
  void slotUpdateDesignPointTable();

  void slotRefresh(int tabID);
  void slotEnableDisableDependentWorkflowTabs(int tabID, bool hasLogMessage);
  void slotDisableTab();

  void slotResetTab();
  void slotUpdateIterationDir();

signals:
  void signalRefresh();

  void signalEnableDependentWorkflowTabs();
  void signalDisableDependentWorkflowTabs();

  void signalEnableDisableDependentTabRunCasaButton(int tabID, bool isEnabled);

private:
  DoeTab* doeTab_;
  UAScenario& casaScenario_;
  ScriptRunController& scriptRunController_;
  InfluentialParameterController* influentialParameterController_;
  ManualDesignPointController* manualDesignPointController_;
};

} // namespace ua

} // namespace casaWizard
