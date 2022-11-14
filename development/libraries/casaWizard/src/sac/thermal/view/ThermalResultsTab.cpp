#include "ThermalResultsTab.h"

#include "view/assets/multiWellPlot.h"
#include "view/assets/wellCorrelationPlotLayout.h"

#include "view/sharedComponents/customradiobutton.h"

#include <QButtonGroup>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QStackedLayout>
#include <QPushButton>
#include <QListWidget>

#include <cmath>
#include "ConstantsNumerical.h"

namespace casaWizard
{

namespace sac
{

namespace thermal
{

ThermalResultsTab::ThermalResultsTab(QWidget* parent) :
   SacResultsTab(parent)
{
   optimizedResultsTable()->setRowCount(0);
   optimizedResultsTable()->setColumnCount(4);

   optimizedResultsTable()->setHorizontalHeaderItem(0, createHeaderItem("Well name", Qt::AlignLeft));
   optimizedResultsTable()->setHorizontalHeaderItem(1, createHeaderItem("Original value", Qt::AlignRight));
   optimizedResultsTable()->setHorizontalHeaderItem(2, createHeaderItem("Optimized value", Qt::AlignRight));
   optimizedResultsTable()->setHorizontalHeaderItem(3, createHeaderItem("Difference", Qt::AlignRight));

   optimizedResultsTable()->setEditTriggers(QAbstractItemView::NoEditTriggers);
   optimizedResultsTable()->verticalHeader()->hide();
   optimizedResultsTable()->setSelectionMode(QAbstractItemView::NoSelection);
   for (int i = 0; i < optimizedResultsTable()->columnCount(); ++i)
   {
      optimizedResultsTable()->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);
   }
}

void ThermalResultsTab::updateOptimizedHeatProductionTable(const QStringList& wellNames,
                                                           const QVector<double>& originalValues,
                                                           const QVector<double>& optimizedValues)
{
   optimizedResultsTable()->clearContents();
   optimizedResultsTable()->setRowCount(0);

   auto doubleToQString = [](double d){return QString::number(d, 'f', 2);};

   for (int i = 0; i < wellNames.size();)
   {
      if (optimizedValues[i] !=  Utilities::Numerical::CauldronNoDataValue)
      {
         double diff = optimizedValues[i] - originalValues[i];
         addRow ( i,
                  {new QTableWidgetItem(wellNames[i]),
                   new QTableWidgetItem(doubleToQString(originalValues[i])),
                   new QTableWidgetItem(doubleToQString(optimizedValues[i])),
                   new QTableWidgetItem(doubleToQString(diff))}
                  );
      }
      else {
         addRow ( i,
                  {new QTableWidgetItem(wellNames[i]),
                   new QTableWidgetItem(doubleToQString(originalValues[i])),
                   new QTableWidgetItem("Unk."),
                   new QTableWidgetItem("N/a")}
                  );
      }
   }
}

void ThermalResultsTab::addRow(int& row, QVector<QTableWidgetItem*> items)
{
   optimizedResultsTable()->setRowCount(row+1);
   for (int i = 0; i < items.size(); ++i)
   {
      optimizedResultsTable()->setItem(row, i, items[i]);
      if (i>1)
      {
         items[i]->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
      }
   }
   row++;
}

void ThermalResultsTab::setPlotType(const int currentIndex)
{
   SacResultsTab::setPlotType(currentIndex);
   if (currentIndex == 2)
   {
      wellsList()->setSelectionMode(QAbstractItemView::ExtendedSelection);
      selectClear()->setVisible(true);
      selectAll()->setVisible(true);
   }
}

} // namespace thermal

} // namespace sac

} // namespace casaWizard
