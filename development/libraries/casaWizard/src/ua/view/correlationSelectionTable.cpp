#include "correlationSelectionTable.h"
#include "model/influentialParameter.h"
#include "view/sharedComponents/customtitle.h"

#include <QVBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QTableWidget>
#include <QSignalBlocker>

namespace casaWizard
{

namespace ua
{

CorrelationSelectionTable::CorrelationSelectionTable(QWidget* parent) :
  QWidget(parent),
  tableParameters_{new QTableWidget(this)}
{
  tableParameters_->setRowCount(0);
  tableParameters_->setColumnCount(2);

  tableParameters_->setHorizontalHeaderItem(0, new QTableWidgetItem("Influential parameter"));
  tableParameters_->setHorizontalHeaderItem(1, new QTableWidgetItem("") );
  tableParameters_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeMode::Stretch );
  tableParameters_->resizeColumnsToContents();

  QVBoxLayout* totalLayout = new QVBoxLayout();

  totalLayout->addWidget(new CustomTitle("Influential parameters"));
  totalLayout->addWidget(tableParameters_, 1);

  setLayout(totalLayout);
}

void CorrelationSelectionTable::updateTable(const QVector<InfluentialParameter*> influentialParameters, const QVector<bool>& isInfluentialParameterSelected)
{
  QSignalBlocker blocker(tableParameters_);
  tableParameters_->clearContents();
  tableParameters_->setRowCount(0);

  std::size_t row = 0;
  for (const InfluentialParameter* parameter : influentialParameters)
  {
    QTableWidgetItem* itemName = new QTableWidgetItem(parameter->nameArgumentBased());
    itemName->setFlags(itemName->flags() & ~Qt::ItemIsEditable);

    QTableWidgetItem* itemCheckBox = new QTableWidgetItem();
    itemCheckBox->data(Qt::CheckStateRole);

    if (isInfluentialParameterSelected[row])
    {
      itemCheckBox->setCheckState(Qt::Checked);
    }
    else
    {
      itemCheckBox->setCheckState(Qt::Unchecked);
    }

    tableParameters_->setRowCount(row + 1);
    tableParameters_->setItem(row, 0, itemName);
    tableParameters_->setItem(row, 1, itemCheckBox);
    ++row;
  }
}

QVector<bool> CorrelationSelectionTable::getStatus() const
{
  const int nRows = tableParameters_->rowCount();
  QVector<bool> status(nRows, false);

  for (int iRow = 0; iRow < nRows; ++iRow)
  {
    status[iRow] = tableParameters_->item(iRow, 1)->checkState() == Qt::Checked;
  }

  return status;
}

QTableWidget* CorrelationSelectionTable::tableParameters() const
{
  return tableParameters_;
}

} // namespace ua

} // namespace casaWizard
