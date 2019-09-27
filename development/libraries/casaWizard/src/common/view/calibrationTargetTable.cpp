#include "calibrationTargetTable.h"

#include <QTableWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QHeaderView>
#include <QSignalBlocker>

namespace casaWizard
{

CalibrationTargetTable::CalibrationTargetTable(QWidget* parent)
  : QTableWidget(parent),
    columnCheckBox_{3}
{
  setRowCount(0);
  setColumnCount(4);
  setHorizontalHeaderItem(0, new QTableWidgetItem("Well name"));
  setHorizontalHeaderItem(1, new QTableWidgetItem("x [m]"));
  setHorizontalHeaderItem(2, new QTableWidgetItem("y [m]"));
  setHorizontalHeaderItem(columnCheckBox_, new QTableWidgetItem("Select well/s") );
  horizontalHeader()->sectionSizeHint(50);
  horizontalHeader()->setStretchLastSection(true);
  horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

int CalibrationTargetTable::checkBoxColumn() const
{
  return columnCheckBox_;
}

void CalibrationTargetTable::updateTable(const QVector<Well>& wells)
{
  QSignalBlocker blocker(this);
  clearContents();
  setRowCount(0);
  std::size_t row = 0;

  for (const Well& well : wells)
  {
    QTableWidgetItem* itemCheckBox = new QTableWidgetItem();
    itemCheckBox->data(Qt::CheckStateRole);
    itemCheckBox->setCheckState(well.isActive() ? Qt::Checked : Qt::Unchecked);

    setRowCount(row + 1);
    setItem(row, 0, new QTableWidgetItem(well.name()));
    setItem(row, 1, new QTableWidgetItem(QString::number(well.x(), 'g', 12)));
    setItem(row, 2, new QTableWidgetItem(QString::number(well.y(), 'g', 12)));
    setItem(row, columnCheckBox_, itemCheckBox);
    ++row;
  }
  resizeColumnsToContents();
}

}  // namespace casaWizard
