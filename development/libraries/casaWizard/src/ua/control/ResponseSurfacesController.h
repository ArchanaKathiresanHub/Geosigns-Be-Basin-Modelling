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
class ResponseSurfacesTab;
class UAScenario;

class ResponseSurfacesController : public QObject
{
  Q_OBJECT

public:
  ResponseSurfacesController(ResponseSurfacesTab* responseSurfacesTab,
               UAScenario& casaScenario,
               ScriptRunController& scriptRunController,
               QObject* parent);

private slots:
  void slotPushButtonQCrunCasaClicked();
  void slotTableQCCurrentItemChanged(QTableWidgetItem* current, QTableWidgetItem* previous);

  void slotModelChange();
  void slotUpdateTabGUI(int tabID);

signals:
  void signalRefreshChildWidgets();

private:
  bool writeCasaScriptSA();

private:
  void refreshGUI();
  int m_targetIndex;

  ResponseSurfacesTab* m_responseSurfacesTab;
  UAScenario& m_casaScenario;
  ScriptRunController& m_scriptRunController;
  QCDoeOptionController* m_qcDoeOptionController;
};

} // namespace ua

} // namespace casaWizard
