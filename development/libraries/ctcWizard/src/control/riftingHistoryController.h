#pragma once

#include <QObject>

class QTableWidgetItem;

namespace ctcWizard
{

class RiftingHistoryTable;
class CtcScenario;

class RiftingHistoryController : public QObject
{
  Q_OBJECT

public:
  explicit RiftingHistoryController(RiftingHistoryTable* table, CtcScenario& ctcScenario, QObject* parent);
      void fillRiftTable();

private slots:
  void slotTableWidgetRiftingHistoryItemChanged(QTableWidgetItem* item);

private:
  void updateRiftingHistoryTable();

private:
  RiftingHistoryTable* table_;
  CtcScenario& ctcScenario_;
};

} // namespace ctcWizard
