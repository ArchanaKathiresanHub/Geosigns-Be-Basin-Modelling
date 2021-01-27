#include "riftingHistoryTable.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

namespace ctcWizard
{

RiftingHistoryTable::RiftingHistoryTable(QWidget* parent) : QWidget(parent),
  tableWidgetRiftingHistory_{new QTableWidget(this)}
{
  tableWidgetRiftingHistory_->setRowCount(0);
  tableWidgetRiftingHistory_->setColumnCount(7);
  tableWidgetRiftingHistory_->setHorizontalHeaderItem(0, new QTableWidgetItem("Age"));
  tableWidgetRiftingHistory_->setHorizontalHeaderItem(1, new QTableWidgetItem("PWD - Y/N"));
  tableWidgetRiftingHistory_->setHorizontalHeaderItem(2, new QTableWidgetItem("Tectonic Flag"));
  tableWidgetRiftingHistory_->setHorizontalHeaderItem(3, new QTableWidgetItem("RDA Adjustment [m]"));
  tableWidgetRiftingHistory_->setHorizontalHeaderItem(4, new QTableWidgetItem("RDA Adjustment - Map"));
  tableWidgetRiftingHistory_->setHorizontalHeaderItem(5, new QTableWidgetItem("Max Basalt Thickness [m]"));
  tableWidgetRiftingHistory_->setHorizontalHeaderItem(6, new QTableWidgetItem("Max Basalt Thickness - Map"));
  QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Expanding);
  sizePolicy1.setHorizontalStretch(1);
  sizePolicy1.setVerticalStretch(1);
  sizePolicy1.setHeightForWidth(tableWidgetRiftingHistory_->sizePolicy().hasHeightForWidth());
  tableWidgetRiftingHistory_->setSizePolicy(sizePolicy1);
  tableWidgetRiftingHistory_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  tableWidgetRiftingHistory_->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
  tableWidgetRiftingHistory_->setSortingEnabled(false);
  tableWidgetRiftingHistory_->setRowCount(0);
  tableWidgetRiftingHistory_->setColumnCount(7);
  tableWidgetRiftingHistory_->horizontalHeader()->setVisible(true);
  tableWidgetRiftingHistory_->horizontalHeader()->setCascadingSectionResizes(true);
  tableWidgetRiftingHistory_->horizontalHeader()->setDefaultSectionSize(150);
  tableWidgetRiftingHistory_->horizontalHeader()->resizeSection(0,50);
  tableWidgetRiftingHistory_->horizontalHeader()->resizeSection(1,80);
  tableWidgetRiftingHistory_->horizontalHeader()->resizeSection(2,120);
  tableWidgetRiftingHistory_->horizontalHeader()->resizeSection(3,150);
  tableWidgetRiftingHistory_->horizontalHeader()->resizeSection(4,150);
  tableWidgetRiftingHistory_->horizontalHeader()->resizeSection(5,200);
  tableWidgetRiftingHistory_->horizontalHeader()->setStretchLastSection(true);
  tableWidgetRiftingHistory_->verticalHeader()->setProperty("showSortIndicator", QVariant(false));


  QVBoxLayout* verticalLayoutRifting = new QVBoxLayout(this);
  verticalLayoutRifting->addWidget(new QLabel("Rifting History", this));
  verticalLayoutRifting->addWidget(tableWidgetRiftingHistory_);
}

void RiftingHistoryTable::updateTable(const QVector<RiftingHistory>& riftingHistorys,
                                        const QStringList& riftingHistoryOptions, const QStringList& riftingHistoryRDAMaps,
                                      const QStringList& riftingHistoryBasaltMaps)
{
  tableWidgetRiftingHistory_->clearContents();
  tableWidgetRiftingHistory_->setRowCount(0);

  int row = 0;
  for (const RiftingHistory& target : riftingHistorys)
  {
    QComboBox* propertyComboBox = new QComboBox(tableWidgetRiftingHistory_);
    propertyComboBox->insertItems(1, riftingHistoryOptions);
    int i =0;
    for (const QString& option : riftingHistoryOptions)
    {
      if (target.TectonicFlag == option)
      {
        propertyComboBox->setCurrentIndex(i);
        break;
      }
      ++i;
    }

    QComboBox* propertyComboBoxRDAMap = new QComboBox(tableWidgetRiftingHistory_);
    propertyComboBoxRDAMap->insertItems(1, riftingHistoryRDAMaps);
    int iii =0;
    for (const QString& option : riftingHistoryRDAMaps)
    {
      if (target.RDA_Map == option)
      {
        propertyComboBoxRDAMap->setCurrentIndex(iii);
        break;
      }
      ++iii;
    }

    QComboBox* propertyComboBoxBasaltMap = new QComboBox(tableWidgetRiftingHistory_);
    propertyComboBoxBasaltMap->insertItems(1, riftingHistoryBasaltMaps);
    int ii =0;
    for (const QString& option : riftingHistoryBasaltMaps)
    {
      if (target.Basalt_Thickness_Map == option)
      {
        propertyComboBoxBasaltMap->setCurrentIndex(ii);
        break;
      }
      ++ii;
    }

    QTableWidgetItem* item = new QTableWidgetItem(target.TectonicFlag);
    connect(propertyComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, [=](){ item->setText(propertyComboBox->currentText()); });

    QTableWidgetItem* itemRDAMap = new QTableWidgetItem(target.RDA_Map);
    connect(propertyComboBoxRDAMap, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, [=](){ itemRDAMap->setText(propertyComboBoxRDAMap->currentText()); });

    QTableWidgetItem* itemBasaltMap = new QTableWidgetItem(target.Basalt_Thickness_Map);
    connect(propertyComboBoxBasaltMap, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, [=](){ itemBasaltMap->setText(propertyComboBoxBasaltMap->currentText()); });

    tableWidgetRiftingHistory_->setRowCount(row+1);
    tableWidgetRiftingHistory_->setCellWidget(row, 2, propertyComboBox);
    tableWidgetRiftingHistory_->setCellWidget(row, 4, propertyComboBoxRDAMap);
    tableWidgetRiftingHistory_->setCellWidget(row, 6, propertyComboBoxBasaltMap);
    tableWidgetRiftingHistory_->setItem(row, 0, new QTableWidgetItem(target.Age));
    tableWidgetRiftingHistory_->setItem(row, 1, new QTableWidgetItem(target.HasPWD));
    tableWidgetRiftingHistory_->setItem(row, 2, item);
    tableWidgetRiftingHistory_->setItem(row, 3, new QTableWidgetItem(target.RDA));
    tableWidgetRiftingHistory_->setItem(row, 4, itemRDAMap);
    tableWidgetRiftingHistory_->setItem(row, 5, new QTableWidgetItem(target.Basalt_Thickness));
    tableWidgetRiftingHistory_->setItem(row, 6, itemBasaltMap);
    ++row;
  }
}

const QTableWidget* RiftingHistoryTable::tableWidgetRiftingHistory() const
{
  return tableWidgetRiftingHistory_;
}


} // namespace ctcWizard

