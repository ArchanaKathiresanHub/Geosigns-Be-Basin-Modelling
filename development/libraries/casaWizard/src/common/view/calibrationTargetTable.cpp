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

  disconnect(horizontalHeader(), SIGNAL(sectionPressed(int)), this, SLOT(selectColumn(int)));
  disconnect(verticalHeader(), SIGNAL(sectionPressed(int)), this, SLOT(selectRow(int)));
}

void CalibrationTargetTable::updateTable(const QVector<const Well*> wells, const QMap<QString, QSet<int>>& propertyNamesPerWell)
{
  QSignalBlocker blocker(this);
  clearContents();
  setRowCount(0);  

  for (int i = columnCount() - 1; i>3; --i)
  {
    removeColumn(i);
  }

  int row = 0;
  for (const Well* well : wells)
  {
    QWidget* checkBoxWidget = new QWidget();
    CustomCheckbox* itemCheckBox = new CustomCheckbox();
    itemCheckBox->setCheckState(well->isActive() ? Qt::Checked : Qt::Unchecked);

    QHBoxLayout* layoutCheckBox = new QHBoxLayout(checkBoxWidget);
    layoutCheckBox->addWidget(itemCheckBox);
    layoutCheckBox->setAlignment(Qt::AlignCenter);
    layoutCheckBox->setContentsMargins(0,0,0,0);

    connect(itemCheckBox, &CustomCheckbox::stateChanged, [=](int state){emit checkBoxChanged(state, well->id());});

    setRowCount(row + 1);

    setCellWidget(row, 0, checkBoxWidget);
    setItem(row, 1, new QTableWidgetItem(well->name()));
    setItem(row, 2, new QTableWidgetItem(QString::number(well->x(), 'g', 12)));
    setItem(row, 3, new QTableWidgetItem(QString::number(well->y(), 'g', 12)));

    if (well->isInvalid())
    {
      itemCheckBox->enable(false);
      for (int column = 1; column < 4; column++)
      {
        QTableWidgetItem* toDisable = item(row, column);
        toDisable->setFlags(toDisable->flags().setFlag(Qt::ItemIsEnabled, false));
      }
    }
    ++row;
  }

  QIcon icon(":/Done.png");
  QPixmap pixmapDone = icon.pixmap(20,20);
  setColumnCount(4 + propertyNamesPerWell.size());
  int col = 4;
  for (const QString& propertyName : propertyNamesPerWell.keys())
  {
    setHorizontalHeaderItem(col, new QTableWidgetItem(propertyName));
    for (const int row : propertyNamesPerWell[propertyName])
    {
      QWidget* checkWidget = new QWidget();
      QLabel* checkItem = new QLabel();
      checkItem->setPixmap(pixmapDone);
      checkItem->setAlignment(Qt::AlignHCenter);

      QHBoxLayout* layoutCheckBox = new QHBoxLayout(checkWidget);
      layoutCheckBox->addWidget(checkItem);
      layoutCheckBox->setAlignment(Qt::AlignCenter);
      layoutCheckBox->setContentsMargins(0,0,0,0);

      setCellWidget(row, col, checkWidget);
    }
    col++;
  }

  update();
}

void CalibrationTargetTable::selectAllWells()
{
  for (int row = 0; row < rowCount(); row++)
  {
    CustomCheckbox* itemCheckBox = static_cast<CustomCheckbox*>(cellWidget(row, 0)->children()[1]);
    if (itemCheckBox->isEnabled())
    {
      itemCheckBox->setCheckState(Qt::Checked);
    }
  }
}

void CalibrationTargetTable::clearWellSelection()
{
  for (int row = 0; row < rowCount(); row++)
  {
    CustomCheckbox* itemCheckBox = static_cast<CustomCheckbox*>(cellWidget(row, 0)->children()[1]);
    if (itemCheckBox->isEnabled())
    {
      itemCheckBox->setCheckState(Qt::Unchecked);
    }
  }
}

QVector<int> CalibrationTargetTable::getWellSelection() const
{
  QVector<int> selectedWells;
  for (int row = 0; row < rowCount(); row++)
  {
    CustomCheckbox* itemCheckBox = static_cast<CustomCheckbox*>(cellWidget(row, 0)->children()[1]);
    if (itemCheckBox->isEnabled() && itemCheckBox->isChecked())
    {
      selectedWells.push_back(row);
    }
  }
  return selectedWells;
}

}  // namespace casaWizard
