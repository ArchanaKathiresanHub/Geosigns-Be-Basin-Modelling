// Controller for the table with the lithofractions, used in the SAC wizard
#pragma once

#include <QObject>

class QTableWidgetItem;

namespace casaWizard
{

class ProjectReader;

namespace sac
{

class LithofractionManager;
class LithofractionTable;
class SACScenario;

class LithofractionController : public QObject
{
  Q_OBJECT

public:
  explicit LithofractionController(LithofractionTable* table, SACScenario& scenario, QObject* parent);
  void updateLithofractionTable();

private slots:
  void slotAddRow();
  void slotDelRow();
  void slotTableChange(QTableWidgetItem* item);  
  void slotLayersFromProject();

private:
  const ProjectReader& projectReader_;
  LithofractionManager& lithofractionManager_;
  LithofractionTable* table_;
};

} // namespace sac

} // namespace casaWizard
