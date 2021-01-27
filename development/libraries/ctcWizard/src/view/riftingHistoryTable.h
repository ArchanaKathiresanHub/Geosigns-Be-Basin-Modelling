#pragma once

#include "model/riftingHistory.h"

#include <QWidget>

class QPushButton;
class QTableWidget;

namespace ctcWizard
{

class RiftingHistoryTable : public QWidget
{
  Q_OBJECT

public:
  explicit RiftingHistoryTable(QWidget* parent = 0);

  void updateTable(const QVector<RiftingHistory>& riftingHistory,
                   const QStringList& riftingHistoryOptions, const QStringList& riftingHistoryRDAMaps,
                   const QStringList& riftingHistoryBasaltMaps);

  const QTableWidget* tableWidgetRiftingHistory() const;


private:
  QTableWidget* tableWidgetRiftingHistory_;
};

} // namespace ctcWizard
