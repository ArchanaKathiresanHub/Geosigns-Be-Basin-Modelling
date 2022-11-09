#include "LithologyResultsTab.h"

#include "view/assets/multiWellPlot.h"
#include "view/assets/wellCorrelationPlotLayout.h"

#include "model/lithofraction.h"
#include "model/optimizedLithofraction.h"

#include "view/sharedComponents/customradiobutton.h"

#include <QButtonGroup>
#include <QHeaderView>
#include <QListWidget>
#include <QPushButton>
#include <QStackedLayout>
#include <QTableWidget>

#include <math.h>

namespace casaWizard
{

namespace sac
{

namespace lithology
{

LithologyResultsTab::LithologyResultsTab(QWidget* parent) :
   SacResultsTab(parent),
   wellPrepOrSAC_{new QButtonGroup(this)}
{
   CustomRadioButton* sacDataButton = new CustomRadioButton("SAC data", this);
   sacDataButton->setChecked(true);
   CustomRadioButton* wellPrepDataButton = new CustomRadioButton("WellPrep data", this);

   wellPrepOrSAC_->addButton(sacDataButton, 0);
   wellPrepOrSAC_->addButton(wellPrepDataButton, 1);
   QHBoxLayout* dataLayout = new QHBoxLayout();
   dataLayout->addWidget(sacDataButton);
   dataLayout->addWidget(wellPrepDataButton);

   layoutWellList()->insertLayout(0, dataLayout);

   optimizedResultsTable()->setRowCount(0);
   optimizedResultsTable()->setColumnCount(5);

   optimizedResultsTable()->setHorizontalHeaderItem(0, createHeaderItem("Layer name", Qt::AlignLeft));
   optimizedResultsTable()->setHorizontalHeaderItem(1, createHeaderItem("Lithofraction", Qt::AlignLeft));
   optimizedResultsTable()->setHorizontalHeaderItem(2, createHeaderItem("Original value", Qt::AlignRight));
   optimizedResultsTable()->setHorizontalHeaderItem(3, createHeaderItem("Optimized value", Qt::AlignRight));
   optimizedResultsTable()->setHorizontalHeaderItem(4, createHeaderItem("Difference", Qt::AlignRight));

   optimizedResultsTable()->setEditTriggers(QAbstractItemView::NoEditTriggers);
   optimizedResultsTable()->verticalHeader()->hide();
   optimizedResultsTable()->setSelectionMode(QAbstractItemView::NoSelection);
   for (int i = 0; i<5; ++i)
   {
      optimizedResultsTable()->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);
   }
}


QButtonGroup* LithologyResultsTab::wellPrepOrSAC() const
{
   return wellPrepOrSAC_;
}

void LithologyResultsTab::updateOptimizedLithoTable(const QStringList& layerNameList,
                                           const QVector<QStringList>& lithoNamesVector,
                                           const QVector<QVector<double>>& originalValuesVector,
                                           const QVector<QVector<double>>& optimizedValuesVector)
{
   optimizedResultsTable()->clearContents();
   optimizedResultsTable()->setRowCount(0);

   auto doubleToQString = [](double d){return QString::number(d, 'f',2); };

   int row = 0;
   for (int i = 0; i<layerNameList.size(); ++i)
   {
      double diff =  optimizedValuesVector[i][0] - originalValuesVector[i][0];
      addLithofractionRow(row, {new QTableWidgetItem(layerNameList[i]),
                                new QTableWidgetItem(lithoNamesVector[i][0]),
                                new QTableWidgetItem(doubleToQString(originalValuesVector[i][0])),
                                new QTableWidgetItem(doubleToQString(optimizedValuesVector[i][0])),
                                new QTableWidgetItem(doubleToQString(diff))}, diff);


      for (int j = 1; j<lithoNamesVector[i].size(); ++j)
      {
         diff = optimizedValuesVector[i][j] - originalValuesVector[i][j];
         addLithofractionRow(row, {nullptr,
                                   new QTableWidgetItem(lithoNamesVector[i][j]),
                                   new QTableWidgetItem(doubleToQString(originalValuesVector[i][j])),
                                   new QTableWidgetItem(doubleToQString(optimizedValuesVector[i][j])),
                                   new QTableWidgetItem(doubleToQString(diff))}, diff);
      }
   }
}

void LithologyResultsTab::addLithofractionRow(int& row, QVector<QTableWidgetItem*> items, const double diff)
{
   optimizedResultsTable()->setRowCount(row+1);
   for (int i = 0; i<items.size();++i)
   {
      optimizedResultsTable()->setItem(row, i, items[i]);
      if (i>1)
      {
         items[i]->setTextAlignment(Qt::AlignRight);
      }
      if (i==4 && std::fabs(diff) > 10)
      {
         items[i]->setBackground(QColor(Qt::red));
      }
   }
   row++;
}

} // namespace lithology

} // namespace sac

} // namespace casaWizard
