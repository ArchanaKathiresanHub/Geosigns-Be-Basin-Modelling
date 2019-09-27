// Controller for the table containing the prediction target. It contains the x, y coordinates, the layer (the top will be tracked) and the variable (like temperature)
#pragma once

#include <QObject>

class QTableWidgetItem;

namespace casaWizard
{

namespace ua
{

class SurfaceTargetTable;
class PredictionTargetManager;
class ProjectReader;

class SurfaceTargetController : public QObject
{
  Q_OBJECT

public:
  explicit SurfaceTargetController(SurfaceTargetTable* table,
                                   PredictionTargetManager& manager,
                                   QObject* parent);

private slots:
  void slotPushButtonAddSurfaceTargetClicked();
  void slotPushButtonDelSurfaceTargetClicked();
  void slotPushButtonCopySurfaceTargetClicked();
  void slotTableWidgetSurfaceTargetsItemChanged(QTableWidgetItem* item);
  void slotSurfaceTargetCheckBoxStateChanged(QTableWidgetItem* item);

  void slotRefresh();

private:
  SurfaceTargetTable* table_;
  PredictionTargetManager& manager_;
};

} // namespace ua

} // namespace casaWizard
