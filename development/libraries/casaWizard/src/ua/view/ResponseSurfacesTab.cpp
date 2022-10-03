#include "ResponseSurfacesTab.h"

#include "model/targetQC.h"
#include "qcDoeOptionTable.h"
#include "view/plot/qcPlot.h"
#include "view/sharedComponents/customtitle.h"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QSignalBlocker>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QVariant>

#include <algorithm>

namespace casaWizard
{

namespace ua
{

ResponseSurfacesTab::ResponseSurfacesTab(QWidget* parent) : QWidget(parent),
   pushButtonQCrunCASA_{new QPushButton("Calculate response surfaces", this)},
   tableQC_{new QTableWidget(this)},
   qcPlot_{new QCPlot(this)},
   qcDoeOptionTable_{new QCDoeOptionTable(this)}
{
   tableQC_->setRowCount(0);
   tableQC_->setColumnCount(5);
   tableQC_->setHorizontalHeaderItem(0, new QTableWidgetItem("ID"));
   tableQC_->setHorizontalHeaderItem(1, new QTableWidgetItem("Target"));
   tableQC_->setHorizontalHeaderItem(2, new QTableWidgetItem("R2"));
   tableQC_->setHorizontalHeaderItem(3, new QTableWidgetItem("R2_adj"));
   tableQC_->setHorizontalHeaderItem(4, new QTableWidgetItem("Q2"));
   QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Preferred);
   sizePolicy1.setHorizontalStretch(1);
   sizePolicy1.setVerticalStretch(0);
   tableQC_->setSizePolicy(sizePolicy1);
   tableQC_->setSortingEnabled(false);
   tableQC_->horizontalHeader()->setVisible(true);
   tableQC_->horizontalHeader()->setCascadingSectionResizes(true);
   tableQC_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
   tableQC_->horizontalHeader()->setStretchLastSection(true);
   tableQC_->verticalHeader()->setProperty("showSortIndicator", QVariant(false));

   QVBoxLayout* DoEOptionTableLayout = new QVBoxLayout();
   DoEOptionTableLayout->addWidget(new CustomTitle("Response surface settings", this));
   DoEOptionTableLayout->addWidget(qcDoeOptionTable_);

   QGridLayout* gridLayout = new QGridLayout();
   gridLayout->addWidget(tableQC_, 0, 0, 2, 1);
   gridLayout->addWidget(qcPlot_, 0, 1, 2, 2);
   gridLayout->addLayout(DoEOptionTableLayout, 2, 0);
   gridLayout->addWidget(pushButtonQCrunCASA_, 2, 2);
   gridLayout->setColumnStretch(1, 1);
   gridLayout->setRowStretch(2, 1);
   setLayout(gridLayout);
}

const QPushButton* ResponseSurfacesTab::pushButtonQCrunCASA() const
{
   return pushButtonQCrunCASA_;
}

void ResponseSurfacesTab::fillQCtable(const QVector<TargetQC>& targetQCs)
{
   QSignalBlocker blocker(tableQC_);
   tableQC_->clearContents();
   tableQC_->setRowCount(0);
   tableQC_->setRowCount(targetQCs.size());

   int row = 0;
   for (const TargetQC& l : targetQCs)
   {
      const QStringList list = QStringList() << QString::number(l.id())
                                             << l.name()
                                             << QString::number(l.R2(),'f',4)
                                             << QString::number(l.R2Adj(),'f',4)
                                             << QString::number(l.Q2(),'f',4);
      int col = 0;
      for (const QString& data : list)
      {
         QTableWidgetItem* item = new QTableWidgetItem(data);
         item->setFlags(item->flags() & ~Qt::ItemIsEditable);
         tableQC_->setItem(row, col, item);
         ++col;
      }
      ++row;
   }
   tableQC_->setCurrentCell(0,0);
   tableQC_->resizeColumnsToContents();
}

void ResponseSurfacesTab::updateQCPlot(const TargetQC& targetQC, const int& tableIndex)
{
   tableQC()->selectRow(tableIndex);

   qcPlot_->setTargetQC(targetQC);
   qcPlot_->update();

   qcPlot_->setXLabel(targetQC.property() + " " + targetQC.unitSI() + " (simulation value)");
   qcPlot_->setYLabel(targetQC.property() + " " + targetQC.unitSI() + " (response surface value)");
   qcPlot_->setFontStyle(FontStyle::small);
}

void ResponseSurfacesTab::resetQCPlot()
{
   qcPlot_->reset();
}

QCDoeOptionTable* ResponseSurfacesTab::qcDoeOptionTable() const
{
   return qcDoeOptionTable_;
}

QTableWidget* ResponseSurfacesTab::tableQC() const
{
   return tableQC_;
}

void ResponseSurfacesTab::allowModification(bool allow)
{
   pushButtonQCrunCASA_->setEnabled(allow);
   qcDoeOptionTable_->setEnabled(allow);
}

} // namespace ua

} // namespace casaWizard
