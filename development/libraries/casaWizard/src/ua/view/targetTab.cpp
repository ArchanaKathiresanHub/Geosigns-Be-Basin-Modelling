#include "targetTab.h"

#include "view/calibrationTargetTable.h"
#include "view/objectiveFunctionTable.h"
#include "view/depthTargetTable.h"
#include "view/surfaceTargetTable.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

namespace casaWizard
{

namespace ua
{

TargetTab::TargetTab(QWidget* parent) :
  QWidget(parent),
  depthTargetTable_{new DepthTargetTable(this)},
  calibrationTargetTable_{new CalibrationTargetTable(this)},
  objectiveFunctionTable_{new ObjectiveFunctionTable(this)},
  surfaceTargetTable_{new SurfaceTargetTable(this)},
  lineEditCalibration_{new QLineEdit(this)},
  pushSelectCalibration_{new QPushButton("Select", this)}
{
  QHBoxLayout* selectionLayout = new QHBoxLayout();
  selectionLayout->addWidget(new QLabel("Calibration targets", this));
  selectionLayout->addWidget(lineEditCalibration_);
  selectionLayout->addWidget(pushSelectCalibration_);

  QGridLayout* tablesLayout = new QGridLayout();
  tablesLayout->addWidget(new QLabel("Wells", this), 0, 0);
  tablesLayout->addWidget(new QLabel("Prediction targets", this), 0, 1);
  tablesLayout->addWidget(new QLabel("Objective function", this), 0, 2);
  tablesLayout->setColumnStretch(0, 2);
  tablesLayout->setColumnStretch(1, 3);
  tablesLayout->setColumnStretch(2, 2);

  QVBoxLayout* predictionTargetTables = new QVBoxLayout();
  predictionTargetTables->addWidget(depthTargetTable_);
  predictionTargetTables->addWidget(surfaceTargetTable_);

  objectiveFunctionTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  tablesLayout->addWidget(calibrationTargetTable_, 1, 0);
  tablesLayout->addLayout(predictionTargetTables, 1, 1);
  //tablesLayout->addWidget(depthTargetTable_,  1, 1);
  tablesLayout->addWidget(objectiveFunctionTable_, 1, 2);

  QVBoxLayout* layout = new QVBoxLayout(this);
  layout->addLayout(selectionLayout);
  layout->addLayout(tablesLayout);
}

CalibrationTargetTable* TargetTab::calibrationTargetTable() const
{
  return calibrationTargetTable_;
}

ObjectiveFunctionTable* TargetTab::objectiveFunctionTable() const
{
  return objectiveFunctionTable_;
}

DepthTargetTable* TargetTab::depthTargetTable() const
{
  return depthTargetTable_;
}

SurfaceTargetTable*TargetTab::surfaceTargetTable() const
{
  return surfaceTargetTable_;
}

QLineEdit* TargetTab::lineEditCalibration() const
{
  return lineEditCalibration_;
}

const QPushButton* TargetTab::pushSelectCalibration() const
{
  return pushSelectCalibration_;
}

} // namespace ua

} // namespace casaWizard
