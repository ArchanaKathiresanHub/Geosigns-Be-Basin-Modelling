#include "calibrationTargetTable.h"

#include <QTableWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QHeaderView>
#include <QSignalBlocker>

#include "view/sharedComponents/customcheckbox.h"

namespace casaWizard
{

CalibrationTargetTable::CalibrationTargetTable(QWidget* parent)
  : QTableWidget(parent)
{
  setRowCount(0);

  horizontalHeader()->sectionSizeHint(100);
  horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

  disconnect(horizontalHeader(), SIGNAL(sectionPressed(int)), this, SLOT(selectColumn(int)));
  disconnect(verticalHeader(), SIGNAL(sectionPressed(int)), this, SLOT(selectRow(int)));

  //Disable cell  highlighting:
  setEditTriggers(QAbstractItemView::NoEditTriggers);
  setFocusPolicy(Qt::NoFocus);
  setSelectionMode(QAbstractItemView::NoSelection);
}

void CalibrationTargetTable::updateTableBase(const QVector<const Well*> wells, const QMap<QString, QSet<int>>& propertyNamesPerWell)
{
   setColumnCount(addEnabledCheckBox() ? 4 : 3);
   if (addEnabledCheckBox())
   {
      setHorizontalHeaderItem(0, new QTableWidgetItem("Enabled"));
   }
   setHorizontalHeaderItem(wellNameColumn(), new QTableWidgetItem("Well name"));
   setHorizontalHeaderItem(wellXColumn(), new QTableWidgetItem("x [m]"));
   setHorizontalHeaderItem(wellYColumn(), new QTableWidgetItem("y [m]"));
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
      setRowCount(row + 1);

      setItem(row, wellNameColumn(), new QTableWidgetItem(well->name()));
      setItem(row, wellXColumn(), new QTableWidgetItem(QString::number(well->x(), 'f', 0)));
      setItem(row, wellYColumn(), new QTableWidgetItem(QString::number(well->y(), 'f', 0)));

      if (addEnabledCheckBox())
      {
         QWidget* checkBoxWidget = new QWidget();
         CustomCheckbox* itemCheckBox = new CustomCheckbox();
         itemCheckBox->setCheckState(well->isActive() ? Qt::Checked : Qt::Unchecked);

         QHBoxLayout* layoutCheckBox = new QHBoxLayout(checkBoxWidget);
         layoutCheckBox->addWidget(itemCheckBox);
         layoutCheckBox->setAlignment(Qt::AlignCenter);
         layoutCheckBox->setContentsMargins(0,0,0,0);

         connect(itemCheckBox, &CustomCheckbox::stateChanged, [=](int state){emit checkBoxChanged(state, well->id());});


         setCellWidget(row, 0, checkBoxWidget);
         if (well->isInvalid())
         {
           itemCheckBox->enable(false);
           for (int column = 1; column < 4; column++)
           {
             QTableWidgetItem* toDisable = item(row, column);
             if (toDisable != nullptr)
             {
                toDisable->setFlags(toDisable->flags().setFlag(Qt::ItemIsEnabled, false));
             }
           }
         }
      }

     ++row;
   }

   if (!propertyNamesPerWell.empty())
   {
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
   }
}

void CalibrationTargetTable::updateTable(const QVector<const Well*> wells, const QMap<QString, QSet<int>>& propertyNamesPerWell)
{
  updateTableBase(wells, propertyNamesPerWell);

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

bool CalibrationTargetTable::addEnabledCheckBox()
{
   return true;
}

int CalibrationTargetTable::wellNameColumn()
{
   return 1;
}

int CalibrationTargetTable::wellXColumn()
{
   return 2;
}

int CalibrationTargetTable::wellYColumn()
{
   return 3;
}

}  // namespace casaWizard
