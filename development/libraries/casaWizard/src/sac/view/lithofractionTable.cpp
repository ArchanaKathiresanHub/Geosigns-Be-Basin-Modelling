//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "lithofractionTable.h"

#include "model/input/projectReader.h"
#include "model/lithofraction.h"
#include "view/lithofractionTableRow.h"

#include "../common/view/components/customcheckbox.h"
#include "../common/view/components/customtitle.h"

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
  table_{new QTableWidget(this)}
{
  setTableHeader();
  setTotalLayout();
}

void LithofractionTable::setTableHeader()
{
  table_->setRowCount(0);
  table_->setColumnCount(10);

  setHeaderTitles();
  setHelpToolTips();
  setHeaderResizeModes();
}

void LithofractionTable::setHeaderTitles()
{
  const QStringList tableHeaders{"Enable \n optimization", "Layer name", "Lithotype 1", "Min [%]", "Max [%]",
                                 "Lithotype 2", "Enable 2nd \n optimization", "Min [-]", "Max [-]", "Lithotype 3"};

  for (int i = 0; i<table_->columnCount(); i++)
  {
    table_->setHorizontalHeaderItem(i, new QTableWidgetItem(tableHeaders[i]));
  }
}

void LithofractionTable::setHelpToolTips()
{
  table_->horizontalHeaderItem(3)->setIcon(QIcon(":/Help.png"));
  table_->horizontalHeaderItem(3)->setToolTip("This percentage is defined as: \n litho1 / (litho1 + litho2 + litho3) * 100");

  table_->horizontalHeaderItem(7)->setIcon(QIcon(":/Help.png"));
  table_->horizontalHeaderItem(7)->setToolTip("This ratio is defined as: \n litho2 / (litho2 + litho3)");
}

void LithofractionTable::setHeaderResizeModes()
{
  resizeColumnsToContents();
  stretchColumns();
}

void LithofractionTable::resizeColumnsToContents()
{
  const std::vector<int> resizeToContentsColumns = {0, 1, 3, 4, 6, 7, 8};
  for (const int column : resizeToContentsColumns)
  {
    table_->horizontalHeader()->setSectionResizeMode(column, QHeaderView::ResizeToContents);
  }
}

void LithofractionTable::stretchColumns()
{
  const std::vector<int> stretchColumns = {2, 5, 9};
  for (const int column : stretchColumns)
  {
    table_->horizontalHeader()->setSectionResizeMode(column, QHeaderView::Stretch);
  }
}

void LithofractionTable::setTotalLayout()
{
  QVBoxLayout* layout = new QVBoxLayout();
  layout->addWidget(new CustomTitle("Lithofractions"));
  layout->addWidget(table_,1);
  setLayout(layout);
}

void LithofractionTable::updateTable(const QVector<Lithofraction>& lithofractions, const ProjectReader& projectReader)
{
  QSignalBlocker blocker(table_);
  table_->clearContents();
  table_->setRowCount(0);

  const QStringList layerNames = projectReader.layerNames();

  int row = 0;
  for (const Lithofraction& lithofraction : lithofractions)
  {
    addRow(row, lithofraction, projectReader, layerNames);
    disableProhibitedComponents(row);
    row++;
  }

}

void LithofractionTable::addRow(const int row, const Lithofraction& lithofraction,
                                const ProjectReader& projectReader, const QStringList& layerNames)
{
  table_->setRowCount(row+1);
  const QStringList lithoTypes = obtainLithoTypes(projectReader, lithofraction, layerNames);
  LithofractionTableRow tableRow(lithofraction, lithoTypes);
  addItemsToTable(row, tableRow);

  connectCheckBoxSignals(row, tableRow);
}

QStringList LithofractionTable::obtainLithoTypes(const ProjectReader& projectReader,
                                                 const Lithofraction& lithofraction,
                                                 const QStringList& layerNames)
{
  const int layerIndex = layerNames.indexOf(lithofraction.layerName());
  return projectReader.lithologyTypesForLayer(layerIndex);
}

void LithofractionTable::addItemsToTable(const int row, const LithofractionTableRow& tableRow)
{
  table_->setCellWidget(row, 0, tableRow.firstOptimizationCheckBox());
  table_->setItem(row, 1, new QTableWidgetItem(tableRow.lithofraction().layerName()));
  table_->setItem(row, 2, tableRow.lithoTypeItems()[0]);
  table_->setItem(row, 3, tableRow.firstOptimizationMinPercentage());
  table_->setItem(row, 4, tableRow.firstOptimizationMaxPercentage());
  table_->setItem(row, 5, tableRow.lithoTypeItems()[1]);
  table_->setCellWidget(row, 6, tableRow.secondOptimizationCheckBox());
  table_->setItem(row, 7, tableRow.secondOptimizationMinFraction());
  table_->setItem(row, 8, tableRow.secondOptimizationMaxFraction());
  table_->setItem(row, 9, tableRow.lithoTypeItems()[2]);
}

void LithofractionTable::connectCheckBoxSignals(const int row, const LithofractionTableRow tableRow)
{
  connect(getCheckBoxFromWidget(tableRow.firstOptimizationCheckBox()), &CustomCheckbox::stateChanged, [=](int state){emit firstOptimizationChanged(state, row);});
  connect(getCheckBoxFromWidget(tableRow.secondOptimizationCheckBox()), &CustomCheckbox::stateChanged, [=](int state){emit secondOptimizationChanged(state, row);});
}

CustomCheckbox* LithofractionTable::getCheckBoxFromWidget(QWidget* widget)
{
  return dynamic_cast<CustomCheckbox*>(widget->children()[0]);
}

void LithofractionTable::disableProhibitedComponents(const int row)
{
  checkOptimizations(row);
  checkNumberOfLithotypes(row);
}

void LithofractionTable::checkOptimizations(const int row)
{
  const std::vector<int> allFieldColumns = {1,2,3,4,5,7,8,9};
  const std::vector<int> secondOptimizationColumns = {7,8};

  const bool doFirstOptimization = static_cast<CustomCheckbox*>(table_->cellWidget(row, 0)->children()[0])->isChecked();
  const bool doSecondOptimization = doFirstOptimization && static_cast<CustomCheckbox*>(table_->cellWidget(row, 6)->children()[0])->isChecked();

  setSecondCheckBoxEnabled(row, doFirstOptimization);
  setFieldsEnabled(row, allFieldColumns, doFirstOptimization);
  setFieldsEnabled(row, secondOptimizationColumns, doSecondOptimization);
}

void LithofractionTable::setFieldsEnabled(const int row, const std::vector<int>& fields, const bool enabled)
{
  for (int column : fields)
  {
    QTableWidgetItem* toDisable = table_->item(row, column);
    toDisable->setFlags(toDisable->flags().setFlag(Qt::ItemIsEnabled, enabled));
  }
}

void LithofractionTable::setSecondCheckBoxEnabled(const int row, const bool enabled)
{
  CustomCheckbox* secondOptimization = dynamic_cast<CustomCheckbox*>(table_->cellWidget(row, 6)->children()[0]);
  if (secondOptimization)
  {
    secondOptimization->enable(enabled);
  }
}

void LithofractionTable::checkNumberOfLithotypes(const int row)
{
  if (lessThanThreeLithoTypes(row))
  {
    getCheckBoxFromWidget(table_->cellWidget(row, 6))->setCheckState(Qt::Unchecked);
    getCheckBoxFromWidget(table_->cellWidget(row, 6))->enable(false);
  }
  if (lessThanTwoLithoTypes(row))
  {
    getCheckBoxFromWidget(table_->cellWidget(row, 0))->setCheckState(Qt::Unchecked);
    getCheckBoxFromWidget(table_->cellWidget(row, 0))->enable(false);
  }
}

bool LithofractionTable::lessThanThreeLithoTypes(const int row) const
{
  return table_->item(row, 9)->text() == "";
}

bool LithofractionTable::lessThanTwoLithoTypes(const int row) const
{
  return table_->item(row, 5)->text() == "";
}

const QTableWidget* LithofractionTable::table() const
{
  return table_;
}

void LithofractionTable::updateLithoPercentagesAndFractions(const QVector<Lithofraction>& lithofractions, int row)
{
  std::vector<int> lithoPercentageFields = {3,4,7,8};
  const Lithofraction lithoFraction = lithofractions[row];
  QStringList percentagesAndFractions{ percentageToQString(lithoFraction.minPercentageFirstComponent()),
                                       percentageToQString(lithoFraction.maxPercentageFirstComponent()),
                                       fractionToQString(lithoFraction.minFractionSecondComponent()),
                                       fractionToQString(lithoFraction.maxFractionSecondComponent())};

  for (int i = 0; i < lithoPercentageFields.size(); i++)
  {
    QTableWidgetItem* toUpdate = table_->item(row, lithoPercentageFields[i]);
    toUpdate->setText(percentagesAndFractions[i]);
  }
}

QString LithofractionTable::percentageToQString(const double d)
{
  return QString::number(d, 'f', 2);
}

QString LithofractionTable::fractionToQString(const double d)
{
  return QString::number(d, 'f', 3);
}

} // namespace sac

} // namespace casaWizard
