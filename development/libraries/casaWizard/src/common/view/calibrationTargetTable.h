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

  void updateTable(const QVector<const Well *> wells, const QMap<QString, QSet<int>>& propertyNamesPerWell);

  void selectAllWells();
  void clearWellSelection();

  QVector<int> getWellSelection() const;

signals:
  void checkBoxChanged(int, int);

private:
  void stretchColumns();

};

}  // namespace casaWizard
