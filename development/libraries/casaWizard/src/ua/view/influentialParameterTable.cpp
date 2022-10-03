#include "influentialParameterTable.h"

#include "view/sharedComponents/customtitle.h"
#include "view/tableRowPushButton.h"
#include "view/tableRowComboBox.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QSignalBlocker>

namespace casaWizard
{

namespace ua
{

InfluentialParameterTable::InfluentialParameterTable(QWidget *parent) :
  QWidget(parent),
  tableWidgetInfluentialParameter_{new QTableWidget(this)},
  pushButtonAddInfluentialParameter_{new QPushButton("add", this)},
  pushButtonDelInfluentialParameter_{new QPushButton("del", this)}
{
  tableWidgetInfluentialParameter_->setRowCount(0);
  tableWidgetInfluentialParameter_->setColumnCount(2);
  tableWidgetInfluentialParameter_->setHorizontalHeaderItem(0, new QTableWidgetItem("Influential parameter"));
  tableWidgetInfluentialParameter_->setHorizontalHeaderItem(1, new QTableWidgetItem("Argument list"));
  QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Expanding);
  sizePolicy1.setHorizontalStretch(1);
  sizePolicy1.setVerticalStretch(1);
  sizePolicy1.setHeightForWidth(tableWidgetInfluentialParameter_->sizePolicy().hasHeightForWidth());
  tableWidgetInfluentialParameter_->setSizePolicy(sizePolicy1);
  tableWidgetInfluentialParameter_->setSortingEnabled(false);
  tableWidgetInfluentialParameter_->horizontalHeader()->setVisible(true);
  tableWidgetInfluentialParameter_->horizontalHeader()->setCascadingSectionResizes(true);
  tableWidgetInfluentialParameter_->horizontalHeader()->setStretchLastSection(true);
  tableWidgetInfluentialParameter_->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
  tableWidgetInfluentialParameter_->horizontalHeader()->setDefaultSectionSize(550);
  tableWidgetInfluentialParameter_->verticalHeader()->setProperty("showSortIndicator", QVariant(false));


  QVBoxLayout* buttonsLayout = new QVBoxLayout();
  buttonsLayout->addWidget(new QWidget(parent), 1);
  buttonsLayout->addWidget(pushButtonAddInfluentialParameter_);
  buttonsLayout->addWidget(pushButtonDelInfluentialParameter_);

  QHBoxLayout* tableAndButtonsLayout = new QHBoxLayout();
  tableAndButtonsLayout->addWidget(tableWidgetInfluentialParameter_);
  tableAndButtonsLayout->addLayout(buttonsLayout);

  CustomTitle* tableHeader = new CustomTitle("Parameters and Uncertainty ranges to explore", this);
  QVBoxLayout* totalLayout = new QVBoxLayout(this);
  totalLayout->addWidget(tableHeader);
  totalLayout->addLayout(tableAndButtonsLayout);
}

void InfluentialParameterTable::updateTable(const QVector<InfluentialParameter*> influentialParameters,
                                            const QStringList& defaultParameters)
{
  QSignalBlocker blocker(tableWidgetInfluentialParameter_);
  tableWidgetInfluentialParameter_->clearContents();
  tableWidgetInfluentialParameter_->setRowCount(0);

  int row = 0;
  for (const InfluentialParameter* influentialParameter : influentialParameters)
  {
    tableWidgetInfluentialParameter_->setRowCount(row + 1);
    
    TableRowComboBox* comboBoxInfluentialParameter = new TableRowComboBox{row, tableWidgetInfluentialParameter_};
    int selectedIndex = defaultParameters.indexOf(influentialParameter->name());

    comboBoxInfluentialParameter->insertItems(0, defaultParameters);
    if (selectedIndex < 0 )
    {
      selectedIndex = 0;
    }

    comboBoxInfluentialParameter->setCurrentIndex(selectedIndex);

    connect(comboBoxInfluentialParameter, SIGNAL(currentIndexChanged(int, int)), this, SIGNAL(changeInfluentialParameter(int, int)));

    TableRowPushButton* pushButtonArguments = new TableRowPushButton{influentialParameter->arguments().toString(), row, tableWidgetInfluentialParameter_};
    tableWidgetInfluentialParameter_->setCellWidget(row, 0, comboBoxInfluentialParameter);
    tableWidgetInfluentialParameter_->setCellWidget(row, 1, pushButtonArguments);
    connect(pushButtonArguments, SIGNAL(clicked(int)), this, SIGNAL(popupArguments(int)));

    ++row;
  }
}

const QPushButton* InfluentialParameterTable::pushButtonAddInfluentialParameter() const
{
  return pushButtonAddInfluentialParameter_;
}

const QPushButton* InfluentialParameterTable::pushButtonDelInfluentialParameter() const
{
  return pushButtonDelInfluentialParameter_;
}


const QTableWidget* InfluentialParameterTable::tableWidgetInfluentialParameter() const
{
  return tableWidgetInfluentialParameter_;
}

} // namespace ua

} // namespace casaWizard
