#pragma once

#include <QObject>
#include <QVector>

class QString;
class QTableWidgetItem;

namespace ctcWizard
{

class CtcScenario;
class Controller;
class CTCtab;

class CTCcontroller : public QObject
{
  Q_OBJECT

public:
  CTCcontroller(CTCtab* ctcTab,
                CtcScenario& ctcScenario,
                Controller* mainController);

private slots:
  void slotPushSelectProject3dClicked();
  void slotPushButtonCTCrunCtcClicked();
  void slotLineEditProject3dTextChanged(const QString& project3dPath);
  void slotpushButtonRunFastCauldronClicked();
  void slotlineEditNumProcTextChanged(const QString& numProc);
  void slotRunModeComboBoxChanged(const QString& runMode);
  void slotpushButtonCTCoutputMapsClicked();
  void slotpushButtonExportCTCoutputMapsClicked();

private:
  QString createProject3dwithCTCUIinputs(const QString& scenarioFolderPath);
  void updateProjectTxtFile(const QString& scenarioFolderPath);
  bool checkfastCauldronResultsAvailable(const QString& fastcldrnRunMode);
  bool validateCTCinputFields();
  void upateProject3dfileToStoreOutputs(const QString& filePath);

  CTCtab* ctcTab_;
  CtcScenario& ctcScenario_;
  Controller* mainController_;
  QVector<QObject*> subControllers_;
};

} // namespace ctcWizard
