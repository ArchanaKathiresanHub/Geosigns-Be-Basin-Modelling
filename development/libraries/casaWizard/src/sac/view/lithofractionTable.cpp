#include "lithofractionTable.h"

#include "model/input/projectReader.h"
#include "model/lithofraction.h"
#include "view/tableRowComboBox.h"

#include <QBoxLayout>
#include <QComboBox>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QSignalBlocker>

namespace casaWizard
{

namespace sac
{

LithofractionTable::LithofractionTable(QWidget* parent) :
  QWidget(parent),
  table_{new QTableWidget(this)},
  addRow_{new QPushButton("Add", this)},
  delRow_{new QPushButton("Delete", this)},
  fillLayers_{new QPushButton("Restore defaults from project 3d", this)}
{
  const int columns{7};
  const QStringList tableHeaders{"Layer name", "Litho 1", "Min", "Max", "Litho 2", "Min", "Max"};
  table_->setRowCount(0);
  table_->setColumnCount(columns);
  for (int i = 0; i<columns; i++)
  {
    table_->setHorizontalHeaderItem(i, new QTableWidgetItem(tableHeaders[i]));
  }
  table_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
  table_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
  table_->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);

  QHBoxLayout* buttons = new QHBoxLayout();
  buttons->addWidget(new QWidget(this), 1);
  buttons->addWidget(fillLayers_);
  buttons->addWidget(addRow_);
  buttons->addWidget(delRow_);

  QVBoxLayout* layout = new QVBoxLayout();
  layout->addWidget(new QLabel("Lithofractions"));
  layout->addWidget(table_,1);
  layout->addLayout(buttons);

  setLayout(layout);
}

void LithofractionTable::updateTable(const QVector<Lithofraction>& lithofractions, const ProjectReader& projectReader)
{
  QSignalBlocker blocker(table_);
  table_->clearContents();
  table_->setRowCount(0);

  int row = 0;
  auto percentageToQString = [](double d){return QString::number(d, 'f',2); };
  auto fractionToQString = [](double d){return QString::number(d, 'f',3); };

  const QStringList layerNames = projectReader.layerNames();

  for (const Lithofraction& lithofraction : lithofractions)
  {
    table_->setRowCount(row+1);

    table_->setItem(row, 0, new QTableWidgetItem(lithofraction.layerName()));

    const int layerIndex = layerNames.indexOf(lithofraction.layerName());
    const QStringList percentNames = projectReader.lithologyTypesForLayer(layerIndex);
    TableRowComboBox* firstPercent = new TableRowComboBox(row, table_);

    firstPercent->insertItems(0, percentNames);

    // QComboBox can not be treated as QTableWidgetItem, therefore, we create both an
    // item and a combobox, and connect them here
    QTableWidgetItem* item1 = new QTableWidgetItem();    
    connect(firstPercent, &TableRowComboBox::currentIndexChanged, [=](){item1->setData(Qt::EditRole, firstPercent->currentIndex()); });

    firstPercent->setCurrentIndex(lithofraction.firstComponent());

    table_->setCellWidget(row, 1, firstPercent);
    table_->setItem(row, 1, item1);

    table_->setItem(row, 2, new QTableWidgetItem(percentageToQString(lithofraction.minPercentageFirstComponent())));
    table_->setItem(row, 3, new QTableWidgetItem(percentageToQString(lithofraction.maxPercentageFirstComponent())));

    TableRowComboBox* secondPercent = new TableRowComboBox(row, table_);
    secondPercent->insertItems(0, percentNames);
    secondPercent->insertItem(3, "None");
    QTableWidgetItem* item2 = new QTableWidgetItem();
    connect(secondPercent, &TableRowComboBox::currentIndexChanged, [=](){ item2->setData(Qt::EditRole, secondPercent->currentIndex()); });

    secondPercent->setCurrentIndex(lithofraction.secondComponent());

    table_->setCellWidget(row, 4, secondPercent);
    table_->setItem(row, 4, item2);

    table_->setItem(row, 5, new QTableWidgetItem(fractionToQString(lithofraction.minFractionSecondComponent())));
    table_->setItem(row, 6, new QTableWidgetItem(fractionToQString(lithofraction.maxFractionSecondComponent())));

    row++;
  }
}

const QTableWidget* LithofractionTable::table() const
{
  return table_;
}

const QPushButton* LithofractionTable::addRow() const
{
  return addRow_;
}

const QPushButton* LithofractionTable::delRow() const
{
  return delRow_;
}

const QPushButton* LithofractionTable::fillLayers() const
{
  return fillLayers_;
}

} // namespace sac

} // namespace casaWizard
