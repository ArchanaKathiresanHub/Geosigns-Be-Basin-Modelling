#pragma once

#include "model/well.h"
#include <QTableWidget>

namespace casaWizard
{

class CalibrationTargetTable : public QTableWidget
{
  Q_OBJECT

public:
  explicit CalibrationTargetTable(QWidget* parent = nullptr);

  void updateTable(const QVector<const Well *> wells);

signals:
  void checkBoxChanged(int, int);

};

}  // namespace casaWizard
