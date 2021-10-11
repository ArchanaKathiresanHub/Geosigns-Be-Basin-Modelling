#include "doeOptionTable.h"

#include <QHeaderView>

namespace casaWizard
{

namespace ua
{

const int columnCheckBox = 0;
const int columnNames = 1;
const int columnIndexNDesignPoints = 2;

DoeOptionTable::DoeOptionTable(QWidget* parent) :
  QTableWidget(parent)
{
  setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

  horizontalHeader()->sectionSizeHint(50);
  horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  QSizePolicy policy = sizePolicy();
  policy.setHorizontalPolicy(QSizePolicy::Policy::Expanding);
  policy.setVerticalPolicy(QSizePolicy::Policy::Expanding);
  setSizePolicy(policy);

  setFixedWidth(271);
  setRowCount(0);
  setColumnCount(3);
  setHorizontalHeaderItem(columnNames, new QTableWidgetItem("Method"));
  setHorizontalHeaderItem(columnIndexNDesignPoints, new QTableWidgetItem("Points"));
  setHorizontalHeaderItem(columnCheckBox, new QTableWidgetItem(""));
}

void DoeOptionTable::updateTable(const QVector<DoeOption*>& doeOptions, const QVector<bool>& isDoeOptionsSelected)
{
  clearContents();
  setRowCount(doeOptions.size());

  int row = 0;
  for (const DoeOption* doeOption : doeOptions)
  {
    QTableWidgetItem* itemCheckBox = new QTableWidgetItem("");
    QTableWidgetItem* itemDoeName = new QTableWidgetItem(doeOption->name());
    QTableWidgetItem* itemNDesignPoints = new QTableWidgetItem(QString::number(doeOption->nDesignPoints()));

    itemDoeName->setFlags(itemDoeName->flags() ^ Qt::ItemIsEditable);

    if (doeOption->hasCalculatedDesignPoints())
    {
      itemNDesignPoints->setTextColor(Qt::gray);
      itemNDesignPoints->setFlags(itemNDesignPoints->flags() ^ Qt::ItemIsEditable);
    }

    setItem(row, columnNames, itemDoeName);
    setItem(row, columnIndexNDesignPoints, itemNDesignPoints);

    itemCheckBox->setCheckState(isDoeOptionsSelected[row] ? Qt::Checked : Qt::Unchecked);

    setItem(row, columnCheckBox, itemCheckBox);

    ++row;
  }
  resizeColumnsToContents();
}

int DoeOptionTable::columnIndexNDesignPointsDoeOptionTable() const
{
  return columnIndexNDesignPoints;
}

int DoeOptionTable::columnCheckBoxDoeOptionTable() const
{
  return columnCheckBox;
}

} // namespace ua

} // namespace casaWizard
