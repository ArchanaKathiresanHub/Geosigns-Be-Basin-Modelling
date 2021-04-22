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
  pushSelectCalibration_{new QPushButton("Select", this)},
  comboBoxApplication_{new QComboBox(this)},
  comboBoxCluster_{new QComboBox(this)},
  pushButtonSACrunCASA_{new EmphasisButton("Run 1D optimization", this)},
  buttonRunOriginal1D_{new QPushButton("Run original 1D", this)},
  buttonRunOriginal3D_{new QPushButton("Run original 3D", this)}
{
  comboBoxApplication_->insertItems(0, {"Iteratively Coupled", "Hydrostatic"});
  comboBoxCluster_->insertItems(0, {"LOCAL", "CLUSTER"});

  QHBoxLayout* layoutProject3D = new QHBoxLayout();
  layoutProject3D->addWidget(new QLabel("Project file", this));
  layoutProject3D->addWidget(lineEditProject3D_);
  layoutProject3D->addWidget(pushSelectProject3D_);

  lineEditProject3D_->setReadOnly(true);

  QHBoxLayout* layoutCalibrationFile = new QHBoxLayout();
  layoutCalibrationFile->addWidget(pushSelectCalibration_);

  QWidget* runOptions = new QWidget(this);
  QGridLayout* layoutOption = new QGridLayout(runOptions);
  layoutOption->addWidget(new CustomTitle("Run Options", this), 0, 0);
  layoutOption->addWidget(new QLabel("Run Mode", this), 1, 0);
  layoutOption->addWidget(comboBoxApplication_, 1, 1);
  layoutOption->addWidget(new QLabel("Run Location", this), 2, 0);
  layoutOption->addWidget(comboBoxCluster_, 2, 1);

  layoutOption->addWidget(pushButtonSACrunCASA_, 3, 0, 1, 2);
  layoutOption->addWidget(buttonRunOriginal1D_, 4, 0, 1, 2);
  layoutOption->addWidget(buttonRunOriginal3D_, 5, 0, 1, 2);
  runOptions->setMaximumHeight(160);
  layoutOption->setMargin(0);

  QGridLayout* layoutTablesAndOptions = new QGridLayout();

  QVBoxLayout* lithofractionLayout = new QVBoxLayout();
  lithofractionLayout->addWidget(new CustomTitle("Lithofractions"));
  lithofractionLayout->addWidget(lithofractionTable_);
  layoutTablesAndOptions->addLayout(lithofractionLayout,0,0,1,3);

  QVBoxLayout* calibrationTargetTableLayout = new QVBoxLayout();
  calibrationTargetTableLayout->addWidget(new CustomTitle("Calibration Targets"));
  calibrationTargetTableLayout->addLayout(layoutCalibrationFile);
  calibrationTargetTableLayout->addWidget(calibrationTargetTable_);

  layoutTablesAndOptions->addLayout(calibrationTargetTableLayout, 1,0);

  QVBoxLayout* objectiveFunctionLayout = new QVBoxLayout();

  objectiveFunctionLayout->addWidget(new CustomTitle("Objective Function"));
  objectiveFunctionLayout->addWidget(objectiveFunctionTable_);

  layoutTablesAndOptions->addLayout(objectiveFunctionLayout,1,1);
  layoutTablesAndOptions->addWidget(runOptions,1,2, Qt::Alignment(Qt::AlignmentFlag::AlignTop));

  QVBoxLayout* verticalLayoutTab = new QVBoxLayout(this);
  verticalLayoutTab->addLayout(layoutProject3D);  
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

const QPushButton* SACtab::pushSelectCalibration() const
{
  return pushSelectCalibration_;
}

const QPushButton* SACtab::pushButtonSACrunCASA() const
{
  return pushButtonSACrunCASA_;
}

const QPushButton* SACtab::buttonRunOriginal1D() const
{
  return buttonRunOriginal1D_;
}

const QPushButton* SACtab::buttonRunOriginal3D() const
{
  return buttonRunOriginal3D_;
}

} // namespace sac

} // namespace casaWizard
