//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "lithofractionTableRow.h"

#include "lithofractionTable.h"
#include "model/lithofraction.h"
#include "../common/view/components/customcheckbox.h"

#include <QHBoxLayout>
#include <QStringList>
#include <QTableWidgetItem>

namespace casaWizard
{

namespace sac
{

LithofractionTableRow::LithofractionTableRow(const Lithofraction& lithofraction, const QStringList& lithoTypes) :
  lithofraction_{lithofraction},
  lithoTypes_{lithoTypes}
{
  createLithoTypeItems();
  createOptimizationCheckBoxes();
  createOptimizationInputFields();
}

void LithofractionTableRow::createLithoTypeItems()
{  
  for ( const int component : {lithofraction_.firstComponent(),
                               lithofraction_.secondComponent(),
                               lithofraction_.thirdComponent()})
  {
    QTableWidgetItem* lithotypeItem = new QTableWidgetItem(lithoTypes_[component]);
    lithotypeItem->setFlags(lithotypeItem->flags() & ~Qt::ItemIsEditable);
    lithoTypeItems_.push_back(lithotypeItem);
  }
}

void LithofractionTableRow::createOptimizationCheckBoxes()
{
  firstOptimizationCheckBox_ = createOptimizationCheckBox();  
  initializeCheckBoxes();
}

void LithofractionTableRow::initializeCheckBoxes()
{
  LithofractionTable::getCheckBoxFromWidget(firstOptimizationCheckBox_)->setCheckState(lithofraction_.doFirstOptimization() ? Qt::Checked : Qt::Unchecked);  
}

QWidget* LithofractionTableRow::createOptimizationCheckBox()
{
  QWidget* optimizationWidget = new QWidget();
  CustomCheckbox* optimizationCheckbox = new CustomCheckbox(optimizationWidget);

  QHBoxLayout *layoutCheckBox = new QHBoxLayout(optimizationWidget);
  layoutCheckBox->addWidget(optimizationCheckbox);
  layoutCheckBox->setAlignment(Qt::AlignCenter);
  layoutCheckBox->setContentsMargins(0,0,0,0);

  return optimizationWidget;
}

void LithofractionTableRow::createOptimizationInputFields()
{
  firstOptimizationMinPercentage_ = new QTableWidgetItem(LithofractionTable::percentageToQString(lithofraction_.minPercentageFirstComponent()));
  firstOptimizationMaxPercentage_ = new QTableWidgetItem(LithofractionTable::percentageToQString(lithofraction_.maxPercentageFirstComponent()));
  secondOptimizationMinFraction_ = new QTableWidgetItem(LithofractionTable::fractionToQString(lithofraction_.minFractionSecondComponent()));
  secondOptimizationMaxFraction_ = new QTableWidgetItem(LithofractionTable::fractionToQString(lithofraction_.maxFractionSecondComponent()));

  firstOptimizationMinPercentage_->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
  firstOptimizationMaxPercentage_->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
  secondOptimizationMinFraction_->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
  secondOptimizationMaxFraction_->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
}

QWidget* LithofractionTableRow::firstOptimizationCheckBox() const
{
  return firstOptimizationCheckBox_;
}

std::vector<QTableWidgetItem*> LithofractionTableRow::lithoTypeItems() const
{
  return lithoTypeItems_;
}

QTableWidgetItem* LithofractionTableRow::firstOptimizationMinPercentage() const
{
  return firstOptimizationMinPercentage_;
}

QTableWidgetItem* LithofractionTableRow::firstOptimizationMaxPercentage() const
{
  return firstOptimizationMaxPercentage_;
}

QTableWidgetItem* LithofractionTableRow::secondOptimizationMinFraction() const
{
  return secondOptimizationMinFraction_;
}

QTableWidgetItem* LithofractionTableRow::secondOptimizationMaxFraction() const
{
  return secondOptimizationMaxFraction_;
}

const Lithofraction& LithofractionTableRow::lithofraction() const
{
  return lithofraction_;
}

} // namespace sac

} // namespace casaWizard

