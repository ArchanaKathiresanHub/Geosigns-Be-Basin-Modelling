// Controller for the sensitivity analysis tab of the thermal wizard
#pragma once

#include <QObject>

class QTableWidgetItem;

namespace casaWizard
{

class ScriptRunController;

namespace ua
{

class QCDoeOptionController;
class QCTab;
class UAScenario;

class QCController : public QObject
{
  Q_OBJECT

public:
  QCController(QCTab* QCTab,
               UAScenario& casaScenario,
               ScriptRunController& scriptRunController,
               QObject* parent);

private slots:
  void slotPushButtonQCrunCasaClicked();
  void slotTableQCCurrentItemChanged(QTableWidgetItem* current, QTableWidgetItem* previous);

  void slotModelChange();
  void slotRefresh();
  void slotUpdateTabGUI(int tabID);

signals:
  void signalRefresh();

private:
  bool writeCasaScriptSA();

private:
  QCTab* QCTab_;
  UAScenario& casaScenario_;
  ScriptRunController& scriptRunController_;
  QCDoeOptionController* qcDoeOptionController_;
};

} // namespace ua

} // namespace casaWizard
