//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "objectiveFunctionTableSAC.h"

#include "model/objectiveFunctionManager.h"
#include "view/components/customcheckbox.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QSignalBlocker>

namespace casaWizard
{

ObjectiveFunctionTableSAC::ObjectiveFunctionTableSAC(QWidget* parent)
  : QTableWidget(parent)
{
  setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

  horizontalHeader()->sectionSizeHint(50);
  horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  QSizePolicy policy = sizePolicy();
  policy.setHorizontalPolicy(QSizePolicy::Policy::Expanding);
  policy.setVerticalPolicy(QSizePolicy::Policy::Expanding);
  setSizePolicy(policy);

  disconnect(horizontalHeader(), SIGNAL(sectionPressed(int)), this, SLOT(selectColumn(int)));
  disconnect(verticalHeader(), SIGNAL(sectionPressed(int)), this, SLOT(selectRow(int)));
}

void ObjectiveFunctionTableSAC::updateTable(const ObjectiveFunctionManager& objectiveFunction)
{
  QSignalBlocker blocker(this);
  clearContents();

  const QStringList variables = objectiveFunction.variablesUserNames();
  const int rowCount = variables.size();

  setRowCount(rowCount);
  setVerticalHeaderLabels(variables);

  setColumnCount(4);
  setHorizontalHeaderLabels({"Enabled", "Absolute error", "Relative error", "Cauldron property name"});

  horizontalHeaderItem(1)->setIcon(QIcon(":/Help.png"));
  horizontalHeaderItem(1)->setToolTip("The uncertainty range is defined as: Absolute Error + Relative Error * Property value");
  horizontalHeaderItem(2)->setIcon(QIcon(":/Help.png"));
  horizontalHeaderItem(2)->setToolTip("The uncertainty range is defined as: Absolute Error + Relative Error * Property value");

  for (int row = 0; row < rowCount; ++row)
  {
    QWidget* checkboxWidget = createEnabledCheckBox(objectiveFunction.enabled(row));
    setCellWidget(row, 0, checkboxWidget);
    connect(dynamic_cast<CustomCheckbox*>(checkboxWidget->children()[0]), &CustomCheckbox::stateChanged, [=](int state){emit enabledStateChanged(state, row);});

    setItem(row, 1, new QTableWidgetItem(QString::number(objectiveFunction.absoluteError(row))));
    setItem(row, 2, new QTableWidgetItem(QString::number(objectiveFunction.relativeError(row))));

    QComboBox* propertySelector = new QComboBox(this);
    propertySelector->addItems({"TwoWayTime", "GammaRay", "BulkDensity", "SonicSlowness",
                                "Pressure", "Temperature", "VRe", "Unknown"});
    QString propertyUserName = objectiveFunction.values()[row].variableUserName();
    connect(propertySelector, &QComboBox::currentTextChanged, [=](const QString& propertyCauldronName){emit selectedPropertyChanged(propertyCauldronName, propertyUserName);});

    if (objectiveFunction.values()[row].variableCauldronName() == "Velocity")
    {
      propertySelector->addItem("Velocity");
    }
    propertySelector->setCurrentText(objectiveFunction.variablesCauldronNames()[row]);

    setCellWidget(row, 3, propertySelector);

    if (objectiveFunction.values()[row].variableCauldronName() == "Velocity" && !objectiveFunction.values()[row].enabled())
    {
      dynamic_cast<CustomCheckbox*>(checkboxWidget->children()[0])->enable(false);
      for (int column = 1; column < 3; column++)
      {
        QTableWidgetItem* toDisable = item(row, column);
        toDisable->setFlags(toDisable->flags().setFlag(Qt::ItemIsEnabled, false));
      }
      propertySelector->setDisabled(true);
    }
  }
}

QWidget* ObjectiveFunctionTableSAC::createEnabledCheckBox(const bool enabled)
{
  QWidget* enabledCheckboxWidget = new QWidget();
  CustomCheckbox* enabledCheckbox = new CustomCheckbox(enabledCheckboxWidget);
  enabledCheckbox->setCheckState(enabled ? Qt::Checked : Qt::Unchecked);

  QHBoxLayout *layoutCheckBox = new QHBoxLayout(enabledCheckboxWidget);
  layoutCheckBox->addWidget(enabledCheckbox);
  layoutCheckBox->setAlignment(Qt::AlignCenter);
  layoutCheckBox->setContentsMargins(0,0,0,0);

  return enabledCheckboxWidget;
}

}  // namespace casaWizard
