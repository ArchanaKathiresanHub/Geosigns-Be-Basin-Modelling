#include "sacTab.h"

#include "view/calibrationTargetTable.h"
#include "view/objectiveFunctionTable.h"
#include "view/lithofractionTable.h"
#include "../common/view/components/customtitle.h"
#include "../common/view/components/emphasisbutton.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QVBoxLayout>

namespace casaWizard
{

namespace sac
{

SACtab::SACtab(QWidget* parent) :
  QWidget(parent),
  pushSelectProject3D_{new QPushButton("Select", this)},
  lineEditProject3D_{new QLineEdit(this)},
  calibrationTargetTable_{new CalibrationTargetTable(this)},
  lithofractionTable_{new LithofractionTable(this)},
  objectiveFunctionTable_{new ObjectiveFunctionTable(this)},
  lineEditCalibration_{new QLineEdit(this)},
  pushSelectCalibration_{new QPushButton("Select", this)},
  comboBoxApplication_{new QComboBox(this)},
  comboBoxCluster_{new QComboBox(this)},
  pushButtonSACrunCASA_{new EmphasisButton("Run 1D optimization", this)}
{
  comboBoxApplication_->insertItems(0, {"Iteratively Coupled", "Hydrostatic"});
  comboBoxCluster_->insertItems(0, {"LOCAL", "CLUSTER"});

  QHBoxLayout* layoutProject3D = new QHBoxLayout();
  layoutProject3D->addWidget(new QLabel("Project file", this));
  layoutProject3D->addWidget(lineEditProject3D_);
  layoutProject3D->addWidget(pushSelectProject3D_);

  QHBoxLayout* layoutCalibrationFile = new QHBoxLayout();
  layoutCalibrationFile->addWidget(new QLabel("Calibration targets", this));
  layoutCalibrationFile->addWidget(lineEditCalibration_);
  layoutCalibrationFile->addWidget(pushSelectCalibration_);

  QWidget* runOptions = new QWidget(this);
  QGridLayout* layoutOption = new QGridLayout(runOptions);
  layoutOption->addWidget(new CustomTitle("Run Options", this), 0, 0);
  layoutOption->addWidget(new QLabel("Run Mode", this), 1, 0);
  layoutOption->addWidget(comboBoxApplication_, 1, 1);
  layoutOption->addWidget(new QLabel("Cluster", this), 2, 0);
  layoutOption->addWidget(comboBoxCluster_, 2, 1);

  layoutOption->addWidget(pushButtonSACrunCASA_, 3, 0, 1, 2);
  runOptions->setMaximumHeight(110);
  layoutOption->setMargin(0);

  QGridLayout* layoutTablesAndOptions = new QGridLayout();
  QVBoxLayout* calibrationTargetTableLayout = new QVBoxLayout();
  calibrationTargetTableLayout->addWidget(new CustomTitle("Calibration Targets"));
  calibrationTargetTableLayout->addWidget(calibrationTargetTable_);

  layoutTablesAndOptions->addLayout(calibrationTargetTableLayout, 0,0);

  QVBoxLayout* objectiveFunctionLayout = new QVBoxLayout();

  objectiveFunctionLayout->addWidget(new CustomTitle("Objective Function"));
  objectiveFunctionLayout->addWidget(objectiveFunctionTable_);

  layoutTablesAndOptions->addLayout(objectiveFunctionLayout,0,1);
  layoutTablesAndOptions->addWidget(runOptions,0,2, Qt::Alignment(Qt::AlignmentFlag::AlignTop));

  QVBoxLayout* verticalLayoutTab = new QVBoxLayout(this);
  verticalLayoutTab->addLayout(layoutProject3D);
  verticalLayoutTab->addLayout(layoutCalibrationFile);
  verticalLayoutTab->addWidget(lithofractionTable_);
  verticalLayoutTab->addLayout(layoutTablesAndOptions);
}

CalibrationTargetTable* SACtab::calibrationTargetTable() const
{
  return calibrationTargetTable_;
}

LithofractionTable* SACtab::lithofractionTable() const
{
  return lithofractionTable_;
}

ObjectiveFunctionTable* SACtab::objectiveFunctionTable() const
{
  return objectiveFunctionTable_;
}

QLineEdit* SACtab::lineEditProject3D() const
{
  return lineEditProject3D_;
}

const QPushButton* SACtab::pushSelectProject3D() const
{
  return pushSelectProject3D_;
}

QComboBox* SACtab::comboBoxApplication() const
{
  return comboBoxApplication_;
}

QComboBox* SACtab::comboBoxCluster() const
{
  return comboBoxCluster_;
}

QLineEdit* SACtab::lineEditCalibration() const
{
  return lineEditCalibration_;
}

const QPushButton* SACtab::pushSelectCalibration() const
{
  return pushSelectCalibration_;
}

const QPushButton* SACtab::pushButtonSACrunCASA() const
{
  return pushButtonSACrunCASA_;
}

} // namespace sac

} // namespace casaWizard
