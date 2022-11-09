//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "UAResultsTargetTable.h"

#include "model/predictionTargetManager.h"
#include "model/UAResultsTargetsData.h"
#include "view/sharedComponents/customcheckbox.h"

#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>

#include <assert.h>

namespace casaWizard
{

namespace ua
{

static const QMap<QString, QString> s_targetOptionsNamesMap({{"Temperature","Temp."}});

UAResultsTargetTable::UAResultsTargetTable(QWidget *parent):
   QTableWidget(parent),
   m_numDefaultCols(5)
{
   QIcon icon(":/Done.png");
   QPixmap pixmapDone = icon.pixmap(20,20);
   m_checkIcon = std::unique_ptr<QIcon>(new QIcon(pixmapDone));

   connect(this, SIGNAL(cellClicked(int,int)), this, SLOT(slotCellClicked(int, int)));

   const QVector<QString>& targetOptions = PredictionTargetManager::predictionTargetOptions();

   setRowCount(0);
   setColumnCount(m_numDefaultCols+targetOptions.size());

   setHorizontalHeaderItem(0, new QTableWidgetItem("Location name"));
   setHorizontalHeaderItem(1, new QTableWidgetItem("X [m]"));
   setHorizontalHeaderItem(2, new QTableWidgetItem("Y [m]"));
   setHorizontalHeaderItem(3, new QTableWidgetItem("Z [m]"));
   setHorizontalHeaderItem(4, new QTableWidgetItem("Surface"));

   QFontMetrics fm(font());

   //Increase width of location name column, according to user feedback
   setColumnWidth(0,fm.horizontalAdvance("Location name")+64);
   horizontalHeader()->setSectionResizeMode(0,QHeaderView::Fixed);

   for (int i = 1; i < m_numDefaultCols; ++i)
   {
      horizontalHeader()->setSectionResizeMode(i,QHeaderView::Stretch);
   }

   int maxHeaderTextWidthOptions(0);
   QVector<QString> optionNamesTable;
   for (int i = 0; i < targetOptions.size(); ++i)
   {
      QString optionName = targetOptions[i];
      if (s_targetOptionsNamesMap.contains(optionName))
      {
         optionNamesTable.push_back(s_targetOptionsNamesMap[optionName]);
      }
      else
      {
         optionNamesTable.push_back(optionName);
      }
      int itemTextWidth = fm.horizontalAdvance(optionNamesTable.back());
      if (itemTextWidth > maxHeaderTextWidthOptions)
      {
         maxHeaderTextWidthOptions = itemTextWidth;
      }
   }

   for (int i = 0; i < optionNamesTable.size(); ++i)
   {
      QString optionName = optionNamesTable[i];
      QTableWidgetItem* it = new QTableWidgetItem(optionName);
      setHorizontalHeaderItem(m_numDefaultCols+i, it);
      setColumnWidth(m_numDefaultCols+i,maxHeaderTextWidthOptions+6);
      horizontalHeader()->setSectionResizeMode(m_numDefaultCols+i,QHeaderView::Fixed);
   }

   //Select whole row when clicking any of the cells in that row:
   setSelectionBehavior(QTableView::SelectRows);

   //Disable cell  highlighting:
   setEditTriggers(QAbstractItemView::NoEditTriggers);
   setFocusPolicy(Qt::NoFocus);

   setSortingEnabled(false);
   horizontalHeader()->setHighlightSections(false); //Don't show column names in bold upon selection.
}

void UAResultsTargetTable::slotCellClicked(int row, int col)
{
   if (!m_disabledRows.contains(row))
   {
      emit enabledCellClicked(row,col);
   }
}

void UAResultsTargetTable::fillTable(const UAResultsTargetsData& targetsData)
{
   clearContents();
   m_disabledRows.clear();

   const QVector<UAResultsTargetData>& data = targetsData.targetData();

   setRowCount(data.size());

   for (int i = 0; i < data.size(); i++)
   {
      const UAResultsTargetData& target = data[i];

      setItem(i, 0, new QTableWidgetItem(target.locationName));
      setItem(i, 1, new QTableWidgetItem(QString::number(target.x, 'f', 0)));
      setItem(i, 2, new QTableWidgetItem(QString::number(target.y, 'f', 0)));
      setItem(i, 3, new QTableWidgetItem(QString::number(target.z, 'f', 0)));
      setItem(i, 4, new QTableWidgetItem(target.surfaceName));

      QFontMetrics fm(font());

      //Set flags to selectable but not editable
      for (int j = 0; j < m_numDefaultCols; j++)
      {
         QTableWidgetItem* it = item(i,j);

         //Add tooltip if cell text doesn't fit in the cell:
         int colWidth = columnWidth(j);

         QString itemtxt= it->text();
         int itemTextWidth = fm.horizontalAdvance(it->text());
         if (itemTextWidth > colWidth-6)
         {
            it->setToolTip(it->text());
         }

         //Enable selection, but disable editing.
         Qt::ItemFlags flags = it->flags();
         flags |= Qt::ItemIsSelectable;
         flags &= ~Qt::ItemIsEditable;
         it->setFlags(flags);
      }

      const QVector<bool>& propertyStates = target.propertyStates;
      assert(propertyStates.size() == columnCount()-m_numDefaultCols);

      for (int j = 0; j < propertyStates.size(); j++)
      {
         if (propertyStates[j])
         {
            setItem(i, m_numDefaultCols+j, new QTableWidgetItem(*m_checkIcon,""));
         }
         else
         {
            setItem(i, m_numDefaultCols+j, new QTableWidgetItem(""));
         }
      }
   }

   if (data.size() > 0)
   {
      setCurrentCell(0,0);
   }
}

QStyleOptionViewItem UAResultsTargetTable::viewOptions() const
{
   QStyleOptionViewItem option = QTableWidget::viewOptions();
   option.decorationAlignment = Qt::AlignBottom | Qt::AlignHCenter;
   option.decorationPosition = QStyleOptionViewItem::Top;
   return option;
}

void UAResultsTargetTable::disableRows(const QVector<int>& rowsToDisable)
{
   int colCount = columnCount();
   int rCount = rowCount();

   for (int row : rowsToDisable)
   {
      if (row > rCount)
      {
         continue;
      }

      m_disabledRows.insert(row);

      for (int col = 0; col < colCount; col++)
      {
         QTableWidgetItem* it = item(row,col);
         if (it)
         {
            it->setFlags(Qt::NoItemFlags);
         }
      }
   }

   if (rowsToDisable.contains(currentRow()))
   {
      //Find first not disabled row:
      for (int row = 0; row < rCount; row++)
      {
         if (!rowsToDisable.contains(row))
         {
            setCurrentCell(row,0);
            emit cellClicked(row,0);
            return;
         }
      }
      clearSelection();
   }

}

void UAResultsTargetTable::enableAllRows()
{
   int colCount = columnCount();
   int rCount = rowCount();

   m_disabledRows.clear();

   for (int row = 0; row < rCount; row++)
   {
      for (int col = 0; col < colCount; col++)
      {
         QTableWidgetItem* it = item(row,col);
         if (!it)
         {
            continue;
         }
         Qt::ItemFlags flags = it->flags();
         flags |= Qt::ItemIsSelectable;
         flags |= Qt::ItemIsEnabled;
         flags &= ~Qt::ItemIsEditable;
         it->setFlags(flags);
      }
   }
}

} //ua
} //casaWizard
