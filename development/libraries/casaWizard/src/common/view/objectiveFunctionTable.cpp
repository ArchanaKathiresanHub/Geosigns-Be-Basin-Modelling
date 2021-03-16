#include "objectiveFunctionTable.h"

#include "model/objectiveFunctionManager.h"

#include <QHeaderView>
#include <QSignalBlocker>

namespace casaWizard
{

ObjectiveFunctionTable::ObjectiveFunctionTable(QWidget* parent)
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

void ObjectiveFunctionTable::updateTable(const ObjectiveFunctionManager& objectiveFunction)
{
  QSignalBlocker blocker(this);
  clearContents();

  const QStringList variables= objectiveFunction.variables();
  const int rowCount = variables.size();

  setRowCount(rowCount);
  setVerticalHeaderLabels(variables);

  setColumnCount(3);
  setHorizontalHeaderLabels({"Absolute", "Relative", "Weight"});

  for (int row = 0; row < rowCount; ++row)
  {
    setItem(row, 0, new QTableWidgetItem(QString::number(objectiveFunction.absoluteError(row))));
    setItem(row, 1, new QTableWidgetItem(QString::number(objectiveFunction.relativeError(row))));
    setItem(row, 2, new QTableWidgetItem(QString::number(objectiveFunction.weight(row))));
  }
}

}  // namespace casaWizard
