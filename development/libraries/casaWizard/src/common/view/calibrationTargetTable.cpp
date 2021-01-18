#include "calibrationTargetTable.h"

#include <QTableWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QHeaderView>
#include <QSignalBlocker>

#include "../common/view/components/customcheckbox.h"

namespace casaWizard
{

CalibrationTargetTable::CalibrationTargetTable(QWidget* parent)
  : QTableWidget(parent)
{
  setRowCount(0);
  setColumnCount(4);
  setHorizontalHeaderItem(0, new QTableWidgetItem("Enabled"));
  setHorizontalHeaderItem(1, new QTableWidgetItem("Well name"));
  setHorizontalHeaderItem(2, new QTableWidgetItem("x [m]"));
  setHorizontalHeaderItem(3, new QTableWidgetItem("y [m]"));

  horizontalHeader()->sectionSizeHint(100);
  horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void CalibrationTargetTable::updateTable(const QVector<const Well*> wells)
{
  QSignalBlocker blocker(this);
  clearContents();
  setRowCount(0);
  int row = 0;

  for (const Well* well : wells)
  {
    QWidget* checkBoxWidget = new QWidget();
    CustomCheckbox* itemCheckBox = new CustomCheckbox();
    itemCheckBox->setCheckState(well->isActive() ? Qt::Checked : Qt::Unchecked);

    QHBoxLayout *layoutCheckBox = new QHBoxLayout(checkBoxWidget);
    layoutCheckBox->addWidget(itemCheckBox);
    layoutCheckBox->setAlignment(Qt::AlignCenter);
    layoutCheckBox->setContentsMargins(0,0,0,0);

    connect(itemCheckBox, &CustomCheckbox::stateChanged, [=](int state){emit checkBoxChanged(state, well->id());});

    setRowCount(row + 1);

    setCellWidget(row, 0, checkBoxWidget);
    setItem(row, 1, new QTableWidgetItem(well->name()));
    setItem(row, 2, new QTableWidgetItem(QString::number(well->x(), 'g', 12)));
    setItem(row, 3, new QTableWidgetItem(QString::number(well->y(), 'g', 12)));

    ++row;
  }
  resizeColumnsToContents();
}

}  // namespace casaWizard
