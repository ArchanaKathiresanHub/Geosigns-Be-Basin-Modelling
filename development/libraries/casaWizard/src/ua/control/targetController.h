// Controller for the target of the thermal wizard
#pragma once

#include <QObject>

namespace casaWizard
{

class CalibrationTargetController;
class ObjectiveFunctionController;

namespace ua
{

class PredictionTargetController;
class TargetTab;
class UAScenario;

class TargetController : public QObject
{
  Q_OBJECT

public:
  explicit TargetController(TargetTab* targetTab,
                            UAScenario& scenario,
                            QObject* parent);

private slots:
  void slotPushSelectCalibrationClicked();
  void slotLineEditCalibrationTextChanged(const QString& calibrationTargetsFilename);

  void refreshGUI();
  void slotUpdateTabGUI(int tabID);

  void slotPushSelectAllVReClicked();
  void slotPushSelectAllTemperaturesClicked();

  void slotPushSelectAllTemperaturePredictionTargetsClicked();
  void slotPushSelectAllVRePredictionTargetsClicked();
  void slotPushSelectAllTimeSeriesPredictionTargetsClicked();
  void slotDataChanged();
signals:
  void signalRefreshChildWidgets();

private:
  UAScenario& scenario_;
  TargetTab* targetTab_;

  PredictionTargetController* predictionTargetController_;
  CalibrationTargetController* calibrationTargetController_;
  ObjectiveFunctionController* objectiveFunctionController_;
};

}  // namespace casaWizard

}  // namespace ua
