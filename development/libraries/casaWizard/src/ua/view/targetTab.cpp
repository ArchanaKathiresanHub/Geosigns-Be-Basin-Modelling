#include "targetTab.h"

#include "view/CalibrationTargetTableUA.h"
#include "view/sharedComponents/customtitle.h"
#include "view/objectiveFunctionTable.h"
#include "view/PredictionTargetTable.h"

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
  calibrationTargetTable_{new CalibrationTargetTableUA(this)},
  objectiveFunctionTable_{new ObjectiveFunctionTable(this)},
  predictionTargetTable_{new PredictionTargetTable(this)},
  lineEditCalibration_{new QLineEdit(this)},
  pushSelectCalibration_{new QPushButton("Select", this)},
  pushSelectAllTemperatures_{new QPushButton("Select all temperatures", this)},
  pushSelectAllVRe_{new QPushButton("Select all VRe's", this)},
  pushSelectAllTargetTemperatures_{new QPushButton("Select all temperatures", this)},
  pushSelectAllTargetVRe_{new QPushButton("Select all VRe's", this)},
  pushSelectAllTargetTimeSeries_{new QPushButton("Select all time series", this)}
{
  QHBoxLayout* selectionLayout = new QHBoxLayout();
  selectionLayout->addWidget(new QLabel("Well data upload", this));
  selectionLayout->addWidget(lineEditCalibration_);
  selectionLayout->addWidget(pushSelectCalibration_);

  QHBoxLayout* tmpLayout = new QHBoxLayout();
  tmpLayout->addWidget(new CustomTitle("Well data", this), 0);
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
  QHBoxLayout* tmpLayout2 = new QHBoxLayout();
  tmpLayout2->addWidget(new CustomTitle("Prediction targets", this), 0);
  tmpLayout2->addWidget(new QWidget(this), 1);
  tmpLayout2->addWidget(pushSelectAllTargetTemperatures_);
  tmpLayout2->addWidget(pushSelectAllTargetVRe_);
  tmpLayout2->addWidget(pushSelectAllTargetTimeSeries_);
  predictionTargetsLayout->addLayout(tmpLayout2);
  predictionTargetsLayout->addWidget(predictionTargetTable_);
  predictionTargetsLayout->setContentsMargins(11, 0, 0, 0);

  QHBoxLayout* layout = new QHBoxLayout(this);
  layout->addLayout(wellsLayout, 1);
  layout->addLayout(predictionTargetsLayout, 2); // The 2 is to stretch the predictionTargetsLayout to twice the size of the wellsLayout
}

CalibrationTargetTable* TargetTab::calibrationTargetTable() const
{
  return calibrationTargetTable_;
}

ObjectiveFunctionTable* TargetTab::objectiveFunctionTable() const
{
  return objectiveFunctionTable_;
}

PredictionTargetTable* TargetTab::surfaceTargetTable() const
{
  return predictionTargetTable_;
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

const QPushButton* TargetTab::pushSelectAllTargetTemperatures() const
{
   return pushSelectAllTargetTemperatures_;
}

const QPushButton* TargetTab::pushSelectAllTargetVRe() const
{
   return pushSelectAllTargetVRe_;
}

const QPushButton* TargetTab::pushSelectAllTargetTimeSeries() const
{
   return pushSelectAllTargetTimeSeries_;
}

} // namespace ua

} // namespace casaWizard
