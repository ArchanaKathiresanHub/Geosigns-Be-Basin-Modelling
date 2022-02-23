#include "inputTab.h"

#include "view/calibrationTargetTable.h"
#include "view/objectiveFunctionTableSAC.h"
#include "view/lithofractionTable.h"
#include "view/components/customtitle.h"
#include "view/components/emphasisbutton.h"
#include "view/components/helpLabel.h"

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

InputTab::InputTab(QWidget* parent) :
  QWidget(parent),
  pushSelectProject3D_{new QPushButton("Select", this)},
  lineEditProject3D_{new QLineEdit(this)},
  calibrationTargetTable_{new CalibrationTargetTable(this)},
  lithofractionTable_{new LithofractionTable(this)},
  objectiveFunctionTable_{new ObjectiveFunctionTableSAC(this)},
  pushSelectAllWells_{new QPushButton("Select all", this)},
  pushClearSelection_{new QPushButton("Deselect all", this)},
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

  QHBoxLayout* layoutCalibrationOptions = new QHBoxLayout();
  layoutCalibrationOptions->addWidget(new CustomTitle("Calibration Targets"));
  layoutCalibrationOptions->addWidget(pushSelectAllWells_);
  layoutCalibrationOptions->addWidget(pushClearSelection_);
  layoutCalibrationOptions->setStretch(0,4);

  QWidget* runOptions = new QWidget(this);
  QGridLayout* layoutOption = new QGridLayout(runOptions);
  layoutOption->addWidget(new CustomTitle("Run Options", this), 0, 0);
  layoutOption->addWidget(new QLabel("Run Mode", this), 1, 0);
  layoutOption->addWidget(comboBoxApplication_, 1, 1);
  layoutOption->addWidget(new QLabel("Run Location", this), 2, 0);
  layoutOption->addWidget(comboBoxCluster_, 2, 1);

  layoutOption->addWidget(pushButtonSACrunCASA_, 3, 0, 1, 2);

  QHBoxLayout* run1D = new QHBoxLayout();
  run1D->addWidget(buttonRunOriginal1D_);
  HelpLabel* helpLabelRun1D = new HelpLabel(this, "For plotting and QC purposes under 'Well log plots and Results' tab");
  run1D->addWidget(helpLabelRun1D);
  layoutOption->addLayout(run1D, 4, 0, 1, 2);

  QHBoxLayout* run3D = new QHBoxLayout();
  run3D->addWidget(buttonRunOriginal3D_);
  HelpLabel* helpLabelRun3D = new HelpLabel(this, "For plotting and QC purposes under 'Well log plots and Results' tab");
  run3D->addWidget(helpLabelRun3D);
  layoutOption->addLayout(run3D, 5, 0, 1, 2);
  layoutOption->setMargin(0);

  runOptions->setMaximumHeight(160);

  QGridLayout* layoutTablesAndOptions = new QGridLayout();

  QVBoxLayout* lithofractionLayout = new QVBoxLayout();
  lithofractionLayout->addWidget(new CustomTitle("Lithofractions"));
  lithofractionLayout->addWidget(lithofractionTable_);
  layoutTablesAndOptions->addLayout(lithofractionLayout,0,0,1,3);

  QVBoxLayout* calibrationTargetTableLayout = new QVBoxLayout();  
  calibrationTargetTableLayout->addLayout(layoutCalibrationOptions);
  calibrationTargetTableLayout->addWidget(calibrationTargetTable_);

  layoutTablesAndOptions->addLayout(calibrationTargetTableLayout, 1,0);

  QVBoxLayout* objectiveFunctionLayout = new QVBoxLayout();

  objectiveFunctionLayout->addWidget(new CustomTitle("Data series and uncertainty ranges"));
  objectiveFunctionLayout->addWidget(objectiveFunctionTable_);

  layoutTablesAndOptions->addLayout(objectiveFunctionLayout,1,1);
  layoutTablesAndOptions->addWidget(runOptions,1,2, Qt::Alignment(Qt::AlignmentFlag::AlignTop));

  QVBoxLayout* verticalLayoutTab = new QVBoxLayout(this);
  verticalLayoutTab->addLayout(layoutProject3D);  
  verticalLayoutTab->addLayout(layoutTablesAndOptions);  
}

CalibrationTargetTable* InputTab::calibrationTargetTable() const
{
  return calibrationTargetTable_;
}

LithofractionTable* InputTab::lithofractionTable() const
{
  return lithofractionTable_;
}

ObjectiveFunctionTableSAC* InputTab::objectiveFunctionTable() const
{
  return objectiveFunctionTable_;
}

QLineEdit* InputTab::lineEditProject3D() const
{
  return lineEditProject3D_;
}

const QPushButton* InputTab::pushSelectProject3D() const
{
  return pushSelectProject3D_;
}

QComboBox* InputTab::comboBoxApplication() const
{
  return comboBoxApplication_;
}

QComboBox* InputTab::comboBoxCluster() const
{
  return comboBoxCluster_;
}

const QPushButton*InputTab::pushSelectAllWells() const
{
  return pushSelectAllWells_;
}

const QPushButton*InputTab::pushClearSelection() const
{
  return pushClearSelection_;
}

const QPushButton* InputTab::pushButtonSACrunCASA() const
{
  return pushButtonSACrunCASA_;
}

const QPushButton* InputTab::buttonRunOriginal1D() const
{
  return buttonRunOriginal1D_;
}

const QPushButton* InputTab::buttonRunOriginal3D() const
{
  return buttonRunOriginal3D_;
}

} // namespace sac

} // namespace casaWizard
