#include "tableRowPushButton.h"

#include <QHBoxLayout>
#include <QPushButton>

namespace casaWizard
{

TableRowPushButton::TableRowPushButton(const QString& text, int tableRow, QWidget* parent) :
  QPushButton(text, parent),
  tableRow_{tableRow}
{
  connect(this, SIGNAL(clicked()), this, SLOT(handleClicked()));
  setStyleSheet("text-align:left; padding-left: 5px");
  setGeometry(geometry());
  setMinimumHeight(30);
}

void TableRowPushButton::handleClicked()
{
  emit clicked(tableRow_);
}

} // namespace casaWizard
