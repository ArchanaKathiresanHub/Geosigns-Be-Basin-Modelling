//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "activeWellsTable.h"

#include <QTableWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QHeaderView>
#include <QSignalBlocker>

#include "../common/view/components/customcheckbox.h"

namespace casaWizard
{

namespace sac
{

ActiveWellsTable::ActiveWellsTable(QWidget* parent)
  : QTableWidget(parent)
{
  setRowCount(0);
  setColumnCount(2);
  setHorizontalHeaderItem(0, new QTableWidgetItem("Used"));

  setHorizontalHeaderItem(1, new QTableWidgetItem("Well name"));
  horizontalHeader()->sectionSizeHint(200);
  horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
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
    connect(itemCheckBox, &CustomCheckbox::clicked, [=](){emit selectedWell(well->name());});

    setRowCount(row + 1);

    setCellWidget(row, 0, checkBoxWidget);
    setItem(row, 1, new QTableWidgetItem(well->name()));
    ++row;
  }
  resizeColumnsToContents();
}

}  // namespace sac

}  // namespace casaWizard