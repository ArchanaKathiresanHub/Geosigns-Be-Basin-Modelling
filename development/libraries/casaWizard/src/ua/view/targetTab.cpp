#include "targetTab.h"

#include "view/CalibrationTargetTableUA.h"
#include "view/components/customtitle.h"
#include "view/depthTargetTable.h"
#include "view/objectiveFunctionTable.h"
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
  calibrationTargetTable_{new CalibrationTargetTableUA(this)},
  objectiveFunctionTable_{new ObjectiveFunctionTable(this)},
  surfaceTargetTable_{new SurfaceTargetTable(this)},
  lineEditCalibration_{new QLineEdit(this)},
  pushSelectCalibration_{new QPushButton("Select", this)},
  pushSelectAllTemperatures_{new QPushButton("Select all temperatures", this)},
  pushSelectAllVRe_{new QPushButton("Select all VRe's", this)}
{
  QHBoxLayout* selectionLayout = new QHBoxLayout();
  selectionLayout->addWidget(new QLabel("Well Data upload", this));
  selectionLayout->addWidget(lineEditCalibration_);
  selectionLayout->addWidget(pushSelectCalibration_);

  QHBoxLayout* tmpLayout = new QHBoxLayout();
  tmpLayout->addWidget(new CustomTitle("Well Data", this), 0);
  tmpLayout->addWidget(new QWidget(this), 1);
  tmpLayout->addWidget(pushSelectAllTemperatures_);
  tmpLayout->addWidget(pushSelectAllVRe_);

  QVBoxLayout* wellsLayout = new QVBoxLayout();
  wellsLayout->addLayout(selectionLayout);
  wellsLayout->addLayout(tmpLayout);
  wellsLayout->addWidget(calibrationTargetTable_);
  wellsLayout->setStretch(2, 100);
  wellsLayout->addWidget(new CustomTitle("Data series and uncertainty ranges", this));
  objectiveFunctionTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  wellsLayout->addWidget(objectiveFunctionTable_);

  QVBoxLayout* predictionTargetsLayout = new QVBoxLayout();
  predictionTargetsLayout->addWidget(new CustomTitle("Prediction targets", this));
  predictionTargetsLayout->addWidget(depthTargetTable_);
  predictionTargetsLayout->addWidget(surfaceTargetTable_);

  QHBoxLayout* layout = new QHBoxLayout(this);
  layout->addLayout(wellsLayout);
  layout->addLayout(predictionTargetsLayout);
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

const QPushButton* TargetTab::pushSelectAllTemperatures() const
{
   return pushSelectAllTemperatures_;
}

const QPushButton* TargetTab::pushSelectAllVRe() const
{
   return pushSelectAllVRe_;
}

} // namespace ua

} // namespace casaWizard
