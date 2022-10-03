//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "StringSelectionTable.h"
#include "view/sharedComponents/customcheckbox.h"

#include <QHBoxLayout>
#include <QHeaderView>

namespace casaWizard
{

StringSelectionTable::StringSelectionTable(QString headerName, QWidget *parent):
   QTableWidget(parent)
{
   //Disable cell  highlighting:
   setEditTriggers(QAbstractItemView::NoEditTriggers);
   setFocusPolicy(Qt::NoFocus);
   setSelectionMode(QAbstractItemView::NoSelection);

   setColumnCount(2);
   setHorizontalHeaderItem(0, new QTableWidgetItem("Selected"));
   setHorizontalHeaderItem(1, new QTableWidgetItem(headerName));
   QHeaderView* header = horizontalHeader();
   header->setSectionResizeMode(0,QHeaderView::ResizeToContents);
   header->setSectionResizeMode(1,QHeaderView::Stretch);
}

void StringSelectionTable::slotSelectAll()
{
   bool allSelected = true;
   for (const auto& state : m_selectionStates)
   {
      if (!state->isChecked()) // Check if all strings are selected
      {
         allSelected = false;
         break;
      }
   }

   // If all strings are selected, deselect all strings.
   // Otherwise, select all wells.
   for (const auto& state : m_selectionStates)
   {
      state->setChecked(!allSelected);
   }
}

void StringSelectionTable::updateTable(const QStringList& strings)
{
   setRowCount(strings.size());
   m_selectionStates.resize(strings.size());

   for (int i = 0; i < strings.size(); i++)
   {
      QWidget* checkBoxWidget = new QWidget();
      CustomCheckbox* itemCheckBox = new CustomCheckbox();
      m_selectionStates[i] = itemCheckBox;
      QHBoxLayout* layoutCheckBox = new QHBoxLayout(checkBoxWidget);
      layoutCheckBox->addWidget(itemCheckBox);
      layoutCheckBox->setAlignment(Qt::AlignCenter);
      layoutCheckBox->setContentsMargins(0,0,0,0);

      setCellWidget(i,0, checkBoxWidget);
      setItem(i, 1, new QTableWidgetItem(strings.at(i)));
   }
}

QVector<bool> StringSelectionTable::selectionStates() const
{
   QVector<bool> selectionStates(m_selectionStates.size(),false);
   for (int i = 0; i < m_selectionStates.size(); i++)
   {
      const CustomCheckbox* checkBox = m_selectionStates[i];
      if (checkBox->isChecked()) selectionStates[i] = true;
   }
   return selectionStates;
}

} // namespace casaWizard
