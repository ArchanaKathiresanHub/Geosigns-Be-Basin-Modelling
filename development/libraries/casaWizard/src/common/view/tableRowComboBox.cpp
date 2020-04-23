#include "tableRowComboBox.h"

#include <QHBoxLayout>

namespace casaWizard
{

TableRowComboBox::TableRowComboBox(int tableRow, QWidget* parent) :
  QComboBox(parent),
  tableRow_{tableRow}
{
  connect(this, SIGNAL(currentIndexChanged(QString)), this, SLOT(handleTextChanged(QString)));
  connect(this, SIGNAL(currentIndexChanged(int)),     this, SLOT(handleIndexChanged(int)));
  setMinimumHeight(30);
}

void TableRowComboBox::wheelEvent(QWheelEvent* /*e*/)
{
}

void TableRowComboBox::handleTextChanged(QString currentText)
{
  emit currentTextChanged(tableRow_, currentText);
}

void TableRowComboBox::handleIndexChanged(int selectedIndex)
{
  emit currentIndexChanged(tableRow_, selectedIndex);
}
} // namespace casaWizard

