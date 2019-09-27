// Controller for the table containing the depth targets. It contains the coordinates and the variable (like temperature)
#pragma once

#include <QObject>

class QTableWidgetItem;

namespace casaWizard
{

namespace ua
{

class DepthTargetTable;
class PredictionTargetManager;

class DepthTargetController : public QObject
{
  Q_OBJECT

public:
  explicit DepthTargetController(DepthTargetTable* table, PredictionTargetManager& manager, QObject* parent);

private slots:
  void slotPushButtonAddDepthTargetClicked();
  void slotPushButtonDelDepthTargetClicked();
  void slotPushButtonCopyDepthTargetClicked();
  void slotTableWidgetDepthTargetsItemChanged(QTableWidgetItem* item);
  void slotDepthTargetCheckBoxStateChanged(QTableWidgetItem* item);

  void slotRefresh();

private:
  DepthTargetTable* table_;
  PredictionTargetManager& manager_;
};

} // namespace ua

} // namespace casaWizard
