#include "CalibrationTargetTableUA.h"

#include "view/sharedComponents/customcheckbox.h"
#include <QHBoxLayout>

namespace casaWizard
{

namespace ua
{

CalibrationTargetTableUA::CalibrationTargetTableUA(QWidget* parent) : CalibrationTargetTable(parent)
{
}

void CalibrationTargetTableUA::updateTable(const QVector<const Well*> wells, const QMap<QString, QSet<int> >& propertyNamesPerWell)
{
    updateTableBase(wells, {});
    int col = columnCount();
    setColumnCount(columnCount() + 2);
    setHorizontalHeaderItem(3, new QTableWidgetItem("Temperature"));
    setHorizontalHeaderItem(4, new QTableWidgetItem("VRe"));
    for (const QString& propertyName : {"Temperature", "VRe"})
    {
      for (int row = 0 ; row < rowCount(); row++)
      {
         QWidget* checkBoxWidget = new QWidget();
         CustomCheckbox* itemCheckBox = new CustomCheckbox();
         if (!propertyNamesPerWell[propertyName].contains(row))
         {
            itemCheckBox->enable(false);
         }
         else
         {
            itemCheckBox->setCheckState(wells[row]->propertyActiveState().value(propertyName, true) ? Qt::Checked : Qt::Unchecked);
         }

         QHBoxLayout* layoutCheckBox = new QHBoxLayout(checkBoxWidget);
         layoutCheckBox->addWidget(itemCheckBox);
         layoutCheckBox->setAlignment(Qt::AlignCenter);
         layoutCheckBox->setContentsMargins(0,0,0,0);

         connect(itemCheckBox, &CustomCheckbox::stateChanged, [=](int state){emit activePropertyCheckBoxChanged(state, row, propertyName);});

        setCellWidget(row, col, checkBoxWidget);
      }
      col++;
    }
    update();
}

bool CalibrationTargetTableUA::addEnabledCheckBox()
{
   return false;
}

int CalibrationTargetTableUA::wellNameColumn()
{
   return 0;
}

int CalibrationTargetTableUA::wellXColumn()
{
   return 1;
}

int CalibrationTargetTableUA::wellYColumn()
{
   return 2;
}

} // namespace ua

} // namespace casaWizard
