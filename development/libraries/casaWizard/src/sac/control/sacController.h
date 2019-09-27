#pragma once

#include <QObject>
#include <QVector>

class QString;
class QTableWidgetItem;


namespace casaWizard
{

class CalibrationTargetController;
class ScriptRunController;
class ObjectiveFunctionController;


namespace sac
{

class CaseExtractor;
class DataExtractionController;
class SACScenario;
class SACtab;
class LithofractionController;

class SACcontroller : public QObject
{
  Q_OBJECT

public:
  SACcontroller(SACtab* sacTab,
                SACScenario& casaScenario,
                ScriptRunController& scriptRunController,
                QObject* parent);

private slots:
  void slotPushSelectProject3dClicked();
  void slotPushSelectCalibrationClicked();
  void slotPushButtonSACrunCasaClicked();

  void slotLineEditProject3dTextChanged(QString project3dPath);
  void slotLineEditCalibrationTextChanged(QString CalibrationTargetsFilePath);
  void slotComboBoxClusterCurrentTextChanged(QString clusterName);
  void slotComboBoxApplicationChanged(QString application);

  void slotRefresh();
  void extractAfterOpen();

signals:
  void signalRefresh();

private:
  SACtab* sacTab_;
  SACScenario& casaScenario_;
  ScriptRunController& scriptRunController_;

  CalibrationTargetController* calibrationTargetController_;
  DataExtractionController* dataExtractionController_;
  LithofractionController* lithofractionController_;
  ObjectiveFunctionController* objectiveFunctionController_;
};

} // namespace sac

} // namespace casaWizard
