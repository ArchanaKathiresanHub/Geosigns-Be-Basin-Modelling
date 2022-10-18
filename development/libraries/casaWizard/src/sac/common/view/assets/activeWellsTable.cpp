//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "activeWellsTable.h"

#include "model/well.h"

#include "view/sharedComponents/customcheckbox.h"

#include <QTableWidget>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QSignalBlocker>

namespace casaWizard
{

namespace sac
{

ActiveWellsTable::ActiveWellsTable(QWidget* parent) : QTableWidget(parent)
{
   setRowCount(0);
   setColumnCount(2);
   setHorizontalHeaderItem(0, new QTableWidgetItem("Used"));
   setHorizontalHeaderItem(1, new QTableWidgetItem("Well name"));
   horizontalHeader()->sectionSizeHint(200);
   horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

   resizeColumnsToContents();
}

void ActiveWellsTable::updateTable(const QVector<const Well*> wells)
{
   QSignalBlocker blocker(this);
   clearContents();
   setRowCount(0);
   int row = 0;

   for (const Well* well : wells)
   {
      QWidget* checkBoxWidget = new QWidget();
      CustomCheckbox* itemCheckBox = new CustomCheckbox();
      itemCheckBox->setCheckState( well->isExcluded() ? Qt::Unchecked : Qt::Checked);

      QHBoxLayout *layoutCheckBox = new QHBoxLayout(checkBoxWidget);
      layoutCheckBox->addWidget(itemCheckBox);
      layoutCheckBox->setAlignment(Qt::AlignCenter);
      layoutCheckBox->setContentsMargins(0,0,0,0);

      connect(itemCheckBox, &CustomCheckbox::stateChanged, [=](int state){emit checkBoxChanged(state, well->id());});
      connect(itemCheckBox, &CustomCheckbox::stateChanged, [=](){emit checkBoxSelectionChanged();});

      setRowCount(row + 1);

      setCellWidget(row, 0, checkBoxWidget);
      setItem(row, 1, new QTableWidgetItem(well->name()));
      ++row;
   }
}

}  // namespace sac

}  // namespace casaWizard
