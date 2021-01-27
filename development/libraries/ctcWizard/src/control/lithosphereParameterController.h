#pragma once

#include <QObject>

class QTableWidgetItem;

namespace ctcWizard
{

class LithosphereParameterTable;
class CtcScenario;
class CTCtab;

class LithosphereParameterController : public QObject
{
  Q_OBJECT

public:
  LithosphereParameterController(LithosphereParameterTable* table, CtcScenario& ctcScenario, QObject* parent);

private slots:
  void slotTableWidgetLithosphereParameterItemChanged(QTableWidgetItem* item);

private:
  void updateLithosphereParameterTable();

private:
  LithosphereParameterTable* table_;
  CtcScenario& ctcScenario_;
};

} // namespace ctcWizard
