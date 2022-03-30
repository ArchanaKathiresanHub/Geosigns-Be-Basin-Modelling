// Controller for the target of the thermal wizard
#pragma once

#include <QObject>

namespace casaWizard
{

class CalibrationTargetController;
class ObjectiveFunctionController;

namespace ua
{

class DepthTargetController;
class SurfaceTargetController;
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

signals:
  void signalRefreshChildWidgets();

private:
  UAScenario& scenario_;
  TargetTab* targetTab_;

  DepthTargetController* depthTargetController_;
  SurfaceTargetController* surfaceTargetController_;
  CalibrationTargetController* calibrationTargetController_;
  ObjectiveFunctionController* objectiveFunctionController_;
};

}  // namespace casaWizard

}  // namespace ua
