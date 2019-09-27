#pragma once

#include "model/well.h"
#include <QTableWidget>

namespace casaWizard
{

class CalibrationTargetTable : public QTableWidget
{
  Q_OBJECT

public:
  explicit CalibrationTargetTable(QWidget* parent = 0);

  void updateTable(const QVector<Well> &wells);
  int checkBoxColumn() const;

private:
  std::size_t columnCheckBox_;
};

}  // namespace casaWizard
