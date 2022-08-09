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
class ModelInputsTab;
class InfluentialParameterController;
class ManualDesignPointController;

class ModelInputsController : public QObject
{
  Q_OBJECT

public:
  ModelInputsController(ModelInputsTab* modelInputsTab,
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
  void slotSpinBoxSubSamplingValueChanged(int subSamplingFactor);
  void slotManualDesignPointsChanged();
  void slotDoeSelectionItemChanged(QTableWidgetItem* item);

  void slotUpdateDesignPointTable();
  void slotUpdateDoeOptionTable();
  void slotUpdateIterationDir();
  void slotUpdateTabGUI(int tabID);

signals:
  void signalRefreshChildWidgets();

private:
  void refreshGUI();
  void setDoEstageIncomplete();

  bool buttonRunAddedCasesShouldBeDisabled();
  bool buttonDoERunShouldBeDisabled();

  ModelInputsTab* m_modelInputsTab;
  UAScenario& m_casaScenario;
  ScriptRunController& m_scriptRunController;
  InfluentialParameterController* m_influentialParameterController;
  ManualDesignPointController* m_manualDesignPointController;
};

} // namespace ua

} // namespace casaWizard
