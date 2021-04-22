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
#include <QTableWidgetItem>
#include <QSignalBlocker>

namespace casaWizard
{

namespace sac
{

LithofractionTable::LithofractionTable(QWidget* parent) :
  QTableWidget(parent)
{
  setTableHeader();
}

void LithofractionTable::setTableHeader()
{
  setRowCount(0);
  setColumnCount(9);

  setHeaderTitles();
  setHelpToolTips();
  setHeaderResizeModes();
  setHeaderAlignment();

  disconnect(horizontalHeader(), SIGNAL(sectionPressed(int)), this, SLOT(selectColumn(int)));
  disconnect(verticalHeader(), SIGNAL(sectionPressed(int)), this, SLOT(selectRow(int)));
}

void LithofractionTable::setHeaderTitles()
{
  const QStringList tableHeaders{"Enable \n optimization", "Layer name", "Lithotype 1", "Min [%]", "Max [%]",
                                 "Lithotype 2", "Min [-]", "Max [-]", "Lithotype 3"};

  for (int i = 0; i<columnCount(); i++)
  {
    setHorizontalHeaderItem(i, new QTableWidgetItem(tableHeaders[i]));
  }
}

void LithofractionTable::setHelpToolTips()
{
  horizontalHeaderItem(3)->setIcon(QIcon(":/Help.png"));
  horizontalHeaderItem(3)->setToolTip("This percentage is defined as: \n litho1 / (litho1 + litho2 + litho3) * 100");

  horizontalHeaderItem(6)->setIcon(QIcon(":/Help.png"));
  horizontalHeaderItem(6)->setToolTip("This ratio is defined as: \n litho2 / (litho2 + litho3)");
}

void LithofractionTable::setHeaderResizeModes()
{
  resizeColumnsToContents();
  stretchColumns();
}

void LithofractionTable::resizeColumnsToContents()
{
  const std::vector<int> resizeToContentsColumns = {0, 1, 3, 4, 6, 7};
  for (const int column : resizeToContentsColumns)
  {
    horizontalHeader()->setSectionResizeMode(column, QHeaderView::ResizeToContents);
  }
}

void LithofractionTable::stretchColumns()
{
  const std::vector<int> stretchColumns = {2, 5, 8};
  for (const int column : stretchColumns)
  {
    horizontalHeader()->setSectionResizeMode(column, QHeaderView::Stretch);
  }
}

void LithofractionTable::setHeaderAlignment()
{
  horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);

  const std::vector<int> rightAlignColumns = {3, 4, 6, 7};
  for (const int column : rightAlignColumns)
  {
    horizontalHeaderItem(column)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
  }

  const std::vector<int> centerAlignColumns = {0};
  for (const int column : centerAlignColumns)
  {
    horizontalHeaderItem(column)->setTextAlignment(Qt::AlignCenter);
  }
}

void LithofractionTable::updateTable(const QVector<Lithofraction>& lithofractions, const ProjectReader& projectReader)
{
  QSignalBlocker blocker(this);
  clearContents();
  setRowCount(0);

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
  setRowCount(row+1);
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
  setCellWidget(row, 0, tableRow.firstOptimizationCheckBox());
  setItem(row, 1, new QTableWidgetItem(tableRow.lithofraction().layerName()));
  setItem(row, 2, tableRow.lithoTypeItems()[0]);
  setItem(row, 3, tableRow.firstOptimizationMinPercentage());
  setItem(row, 4, tableRow.firstOptimizationMaxPercentage());
  setItem(row, 5, tableRow.lithoTypeItems()[1]);  
  setItem(row, 6, tableRow.secondOptimizationMinFraction());
  setItem(row, 7, tableRow.secondOptimizationMaxFraction());
  setItem(row, 8, tableRow.lithoTypeItems()[2]);
}

void LithofractionTable::connectCheckBoxSignals(const int row, const LithofractionTableRow tableRow)
{
  connect(getCheckBoxFromWidget(tableRow.firstOptimizationCheckBox()), &CustomCheckbox::stateChanged, [=](int state){emit firstOptimizationChanged(state, row);});
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
  const std::vector<int> allFieldColumns = {1,2,3,4,5,6,7,8};
  const std::vector<int> secondOptimizationColumns = {6,7};

  const bool doFirstOptimization = static_cast<CustomCheckbox*>(cellWidget(row, 0)->children()[0])->isChecked();
  const bool doSecondOptimization = doFirstOptimization && !lessThanThreeLithoTypes(row);

  setFieldsEnabled(row, allFieldColumns, doFirstOptimization);
  setFieldsEnabled(row, secondOptimizationColumns, doSecondOptimization);
}

void LithofractionTable::setFieldsEnabled(const int row, const std::vector<int>& fields, const bool enabled)
{
  for (int column : fields)
  {
    QTableWidgetItem* toDisable = item(row, column);
    toDisable->setFlags(toDisable->flags().setFlag(Qt::ItemIsEnabled, enabled));
  }
}

void LithofractionTable::checkNumberOfLithotypes(const int row)
{
  if (lessThanTwoLithoTypes(row))
  {
    getCheckBoxFromWidget(cellWidget(row, 0))->setCheckState(Qt::Unchecked);
    getCheckBoxFromWidget(cellWidget(row, 0))->enable(false);
  }
}

bool LithofractionTable::lessThanThreeLithoTypes(const int row) const
{
  return item(row, 8)->text() == "";
}

bool LithofractionTable::lessThanTwoLithoTypes(const int row) const
{
  return item(row, 5)->text() == "";
}

void LithofractionTable::updateLithoPercentagesAndFractions(const QVector<Lithofraction>& lithofractions, int row)
{
  std::vector<int> lithoPercentageFields = {3,4,6,7};
  const Lithofraction lithoFraction = lithofractions[row];
  QStringList percentagesAndFractions{ percentageToQString(lithoFraction.minPercentageFirstComponent()),
                                       percentageToQString(lithoFraction.maxPercentageFirstComponent()),
                                       fractionToQString(lithoFraction.minFractionSecondComponent()),
                                       fractionToQString(lithoFraction.maxFractionSecondComponent())};

  for (int i = 0; i < lithoPercentageFields.size(); i++)
  {
    QTableWidgetItem* toUpdate = item(row, lithoPercentageFields[i]);
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
