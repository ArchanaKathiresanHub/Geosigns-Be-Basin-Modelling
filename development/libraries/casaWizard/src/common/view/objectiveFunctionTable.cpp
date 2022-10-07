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
  horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

  QSizePolicy policy = sizePolicy();
  policy.setHorizontalPolicy(QSizePolicy::Policy::Expanding);
  policy.setVerticalPolicy(QSizePolicy::Policy::Expanding);
  setSizePolicy(policy);
  setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

  disconnect(horizontalHeader(), SIGNAL(sectionPressed(int)), this, SLOT(selectColumn(int)));
  disconnect(verticalHeader(), SIGNAL(sectionPressed(int)), this, SLOT(selectRow(int)));
}

void ObjectiveFunctionTable::updateTable(const ObjectiveFunctionManager& objectiveFunction)
{
  QSignalBlocker blocker(this);
  clearContents();

  const QStringList variables= objectiveFunction.variablesCauldronNames();
  const int rowCount = variables.size();

  setRowCount(rowCount);
  setVerticalHeaderLabels(variables);

  setColumnCount(3);
  setHorizontalHeaderLabels({"Absolute error", "Relative error", "Weight"});
  horizontalHeaderItem(0)->setIcon(QIcon(":/Help.png"));
  horizontalHeaderItem(0)->setToolTip("The uncertainty range is defined as: Absolute Error + Relative Error * Property value");
  horizontalHeaderItem(1)->setIcon(QIcon(":/Help.png"));
  horizontalHeaderItem(1)->setToolTip("The uncertainty range is defined as: Absolute Error + Relative Error * Property value");
  horizontalHeaderItem(2)->setIcon(QIcon(":/Help.png"));
  horizontalHeaderItem(2)->setToolTip("The weight defines to what extent the error of a property with respect to the calibration data\n"
                                      "is taken into account in the Markov Chain Monte Carlo simulation, which is constrained by the\n"
                                      "calibration data.");

  for (int row = 0; row < rowCount; ++row)
  {
    setItem(row, 0, new QTableWidgetItem(QString::number(objectiveFunction.absoluteError(row))));
    setItem(row, 1, new QTableWidgetItem(QString::number(objectiveFunction.relativeError(row))));
    setItem(row, 2, new QTableWidgetItem(QString::number(objectiveFunction.weight(row))));
  }
}

}  // namespace casaWizard
