#include "manualDesignPointTable.h"

#include <QBoxLayout>
#include <QPushButton>
#include <QTableWidget>

namespace casaWizard
{

namespace ua
{

ManualDesignPointTable::ManualDesignPointTable(QWidget* parent) :
  QWidget(parent),
  tableDesignPoints_{new QTableWidget(this)},
  buttonAdd_{new QPushButton("Add", this)},
  buttonDelete_{new QPushButton("Delete", this)}
{
  tableDesignPoints_->setRowCount(0);
  tableDesignPoints_->setColumnCount(0);

  QHBoxLayout* buttonLayout = new QHBoxLayout();
  buttonLayout->addWidget(new QWidget(this), 1);
  buttonLayout->addWidget(buttonAdd_);
  buttonLayout->addWidget(buttonDelete_);

  QVBoxLayout* totalLayout = new QVBoxLayout(this);
  totalLayout->addWidget(tableDesignPoints_);
  totalLayout->addLayout(buttonLayout);  
}

const QPushButton* ManualDesignPointTable::buttonAdd() const
{
  return buttonAdd_;
}

const QPushButton* ManualDesignPointTable::buttonDelete() const
{
  return buttonDelete_;
}

const QTableWidget* ManualDesignPointTable::table() const
{
  return tableDesignPoints_;
}

void ManualDesignPointTable::updateTable(const QVector<QVector<double>> parameters, const QVector<bool> completed, const QStringList headers)
{
  QSignalBlocker blocker(tableDesignPoints_);

  tableDesignPoints_->clearContents();
  tableDesignPoints_->setRowCount(0);
  tableDesignPoints_->setColumnCount(0);

  const int nRows = parameters.size();
  if (nRows == 0)
  {
    return;
  }

  const int nCols = parameters[0].size();
  if (nCols == 0)
  {
    return;
  }

  tableDesignPoints_->setRowCount(nRows);
  tableDesignPoints_->setColumnCount(nCols);

  for (int row = 0; row < nRows; ++row)
  {
    for (int col = 0; col < nCols; ++col)
    {
      QTableWidgetItem* item = new QTableWidgetItem(QString::number(parameters[row][col]));
      if (completed[row])
      {
        item->setFlags(item->flags() ^ Qt::ItemIsEditable);
        item->setTextColor(Qt::gray);
      }
      tableDesignPoints_->setItem(row, col, item);
    }
  }

  setColumnHeaders(headers);
}

void ManualDesignPointTable::setColumnHeaders(const QStringList headers)
{
  const int nCols = tableDesignPoints_->columnCount();
  if (nCols != headers.size())
  {
    return;
  }

  for (int i = 0; i<nCols; ++i)
  {
    tableDesignPoints_->setHorizontalHeaderItem(i, new QTableWidgetItem(headers[i]));
  }
}

} // namespace ua

} // namespace casaWizard
