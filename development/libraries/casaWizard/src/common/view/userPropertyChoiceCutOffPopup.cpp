//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "userPropertyChoiceCutOffPopup.h"

#include "view/sharedComponents/customtitle.h"
#include "view/sharedComponents/customcheckbox.h"

#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMap>
#include <QPair>
#include <QTableWidget>

namespace casaWizard
{

UserPropertyChoiceCutOffPopup::UserPropertyChoiceCutOffPopup(QWidget *parent) :
  UserPropertyChoicePopup(parent),
  defaultsMap_()
{
  setFixedWidth(600);
  fillDefaultsMap();
}

void UserPropertyChoiceCutOffPopup::fillDefaultsMap()
{
  defaultsMap_["BulkDensity"] = {1000.0, 3500.0};
  defaultsMap_["DT_FROM_VP"] = {100.0, 900.0};
  defaultsMap_["GammaRay"] = {0.0, 250.0};
  defaultsMap_["Pressure"] = {0.0, 1.0e10};
  defaultsMap_["SonicSlowness"] = {100.0, 900.0};
  defaultsMap_["Temperature"] = {0.0, 1.0e4};
  defaultsMap_["TwoWayTime"] = {0.0, 15000.0};
  defaultsMap_["TWT_FROM_DT"] = {0.0, 15000.0};
  defaultsMap_["Velocity"] = {1000.0, 7000.0};
  defaultsMap_["VRe"] = {0.0, 100.0};
}

void UserPropertyChoiceCutOffPopup::updateTable(const QStringList& propertyUserNames)
{
  UserPropertyChoicePopup::updateTable(propertyUserNames);

  propertyTable()->setColumnCount(4);
  propertyTable()->setHorizontalHeaderLabels({"Select", "Property", "Cut off min", "Cut off max"});
  for (int row=0; row<propertyUserNames.size(); ++row)
  {
    const QPair<double,double> cutOffPair = defaultsMap_.value(propertyUserNames[row], {0.0, 1.0e10});
    for (int col : {2, 3})
    {
      QDoubleSpinBox* doubleSpinBox = new QDoubleSpinBox();
      doubleSpinBox->setMaximum(1.0e10);
      doubleSpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);

      doubleSpinBox->setValue((col==2) ? cutOffPair.first : cutOffPair.second);
      propertyTable()->setCellWidget(row, col, doubleSpinBox);
    }
  }
}

PropertiesWithCutOffRanges UserPropertyChoiceCutOffPopup::getPropertiesWithCutOffRanges() const
{
  PropertiesWithCutOffRanges propertiesWithCutOffRanges;
  for (int row=0; row<propertyTable()->rowCount(); ++row)
  {
    const QString property = getPropertyIfChecked(row);
    if (!property.isEmpty())
    {
      propertiesWithCutOffRanges[property] = {static_cast<QDoubleSpinBox*>(propertyTable()->cellWidget(row, 2))->value(),
                                              static_cast<QDoubleSpinBox*>(propertyTable()->cellWidget(row, 3))->value()};
    }
  }
  return propertiesWithCutOffRanges;
}

} // namespace casaWizard
