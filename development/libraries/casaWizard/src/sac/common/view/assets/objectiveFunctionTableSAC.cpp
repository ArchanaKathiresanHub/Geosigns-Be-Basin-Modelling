//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "objectiveFunctionTableSAC.h"

#include "model/objectiveFunctionManager.h"
#include "view/sharedComponents/customcheckbox.h"
#include <QHeaderView>
#include <QHBoxLayout>
#include <QSignalBlocker>

namespace casaWizard
{

ObjectiveFunctionTableSAC::ObjectiveFunctionTableSAC(QWidget* parent)
  : ObjectiveFunctionTable(parent)
{  
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

  horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
  setColumnWidth(0, 62);

  horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
  QFontMetrics a(font());
  setColumnWidth(3, a.width("Cauldron property name") + 22);

  for (int row = 0; row < rowCount; ++row)
  {
    QWidget* checkboxWidget = createEnabledCheckBox(objectiveFunction.enabled(row));
    setCellWidget(row, 0, checkboxWidget);
    connect(dynamic_cast<CustomCheckbox*>(checkboxWidget->children()[0]), &CustomCheckbox::stateChanged, [=](int state){emit enabledStateChanged(state, row);});

    setItem(row, 1, new QTableWidgetItem(QString::number(objectiveFunction.absoluteError(row))));
    setItem(row, 2, new QTableWidgetItem(QString::number(objectiveFunction.relativeError(row))));
    QTableWidgetItem* cauldronPropertyName = new QTableWidgetItem(objectiveFunction.variablesCauldronNames()[row]);
    cauldronPropertyName->setFlags(cauldronPropertyName->flags() & ~Qt::ItemIsEditable);
    setItem(row, 3, cauldronPropertyName);

    if (cauldronPropertyName->text() == "Velocity" && !objectiveFunction.values()[row].enabled())
    {
      dynamic_cast<CustomCheckbox*>(checkboxWidget->children()[0])->enable(false);
      for (int column = 1; column < 4; column++)
      {
        QTableWidgetItem* toDisable = item(row, column);
        toDisable->setFlags(toDisable->flags().setFlag(Qt::ItemIsEnabled, false));
      }
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
