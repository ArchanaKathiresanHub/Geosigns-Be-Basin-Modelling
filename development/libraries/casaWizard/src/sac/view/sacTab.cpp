#include "sacTab.h"

#include "view/calibrationTargetTable.h"
#include "view/objectiveFunctionTable.h"
#include "view/lithofractionTable.h"

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
  pushButtonSACrunCASA_{new QPushButton("Run CASA", this)}
{
  comboBoxApplication_->insertItems(0, {"fastcauldron \"-itcoupled\"", "fastcauldron \"-temperature\""});
  comboBoxCluster_->insertItems(0, {"LOCAL", "CLUSTER"});

  QHBoxLayout* layoutProject3D = new QHBoxLayout();
  layoutProject3D->addWidget(new QLabel("Project file", this));
  layoutProject3D->addWidget(lineEditProject3D_);
  layoutProject3D->addWidget(pushSelectProject3D_);

  QHBoxLayout* layoutCalibrationFile = new QHBoxLayout();
  layoutCalibrationFile->addWidget(new QLabel("Calibration targets", this));
  layoutCalibrationFile->addWidget(lineEditCalibration_);
  layoutCalibrationFile->addWidget(pushSelectCalibration_);

  QGridLayout* layoutOption = new QGridLayout();

  layoutOption->addWidget(new QLabel("Application", this), 0, 0);
  layoutOption->addWidget(comboBoxApplication_, 0, 1);
  layoutOption->addWidget(new QLabel("Cluster", this), 1, 0);
  layoutOption->addWidget(comboBoxCluster_, 1, 1);

  layoutOption->addWidget(pushButtonSACrunCASA_, 2, 1);

  QHBoxLayout* layoutTablesAndOptions = new QHBoxLayout();
  layoutTablesAndOptions->addWidget(calibrationTargetTable_,2);
  layoutTablesAndOptions->addWidget(objectiveFunctionTable_,1);
  layoutTablesAndOptions->addLayout(layoutOption);

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
