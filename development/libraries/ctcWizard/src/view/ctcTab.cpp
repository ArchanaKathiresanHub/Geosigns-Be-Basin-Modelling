#include "ctcTab.h"

#include "lithosphereParameterTable.h"
#include "riftingHistoryTable.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QVBoxLayout>

namespace ctcWizard
{

CTCtab::CTCtab(QWidget* parent) :
  QWidget(parent),
  pushSelectProject3D_{new QPushButton("Select", this)},
  lineEditProject3D_{new QLineEdit(this)},
  lineEditNumProc_{new QLineEdit(this)},
  fastcldrnRunModeComboBox_{new QComboBox(this)},
  pushButtonRunFastCauldron_{new QPushButton("Run Cauldron", this)},
  lithosphereParameterTable_{new LithosphereParameterTable(this)},
  riftingHistoryTable_{new RiftingHistoryTable(this)},
  pushButtonCTCrunCTC_{new QPushButton("Run CTC", this)},
  pushButtonCTCoutputMaps_{new QPushButton("View CTC Output Maps", this)},
  pushButtonExportCTCoutputMaps_{new QPushButton("Create Scenario for ALC", this)}
{

  QHBoxLayout* horizontalLayoutProject3D = new QHBoxLayout();
  horizontalLayoutProject3D->setSpacing(6);
  horizontalLayoutProject3D->addWidget(new QLabel("Project.project3D", this));
  horizontalLayoutProject3D->addWidget(lineEditProject3D_);
  horizontalLayoutProject3D->addWidget(pushSelectProject3D_);

  // RunCTC
  QGridLayout* CTCrunGridLayout = new QGridLayout();
  CTCrunGridLayout->setSpacing(6);
  const QSize runBtnCTCSize = QSize(80, 30);
  pushButtonCTCrunCTC_->setFixedSize(runBtnCTCSize);
  pushButtonCTCrunCTC_->setLayoutDirection(Qt::LeftToRight);
  CTCrunGridLayout->addWidget(pushButtonCTCrunCTC_,0,0);

  //View CTC Output Maps
  const QSize viewCTCmapsBtnSize = QSize(170, 30);
  pushButtonCTCoutputMaps_->setFixedSize(viewCTCmapsBtnSize);
  pushButtonCTCoutputMaps_->setLayoutDirection(Qt::LeftToRight);
  CTCrunGridLayout->addWidget(pushButtonCTCoutputMaps_,0,1);

  //Export CTC Output Maps
  const QSize exportCTCmapsBtnSize = QSize(170, 30);
  pushButtonExportCTCoutputMaps_->setFixedSize(exportCTCmapsBtnSize);
  pushButtonExportCTCoutputMaps_->setLayoutDirection(Qt::LeftToRight);
  CTCrunGridLayout->addWidget(pushButtonExportCTCoutputMaps_,0,2);


  QHBoxLayout* horizontalLayout = new QHBoxLayout();
  horizontalLayout->setSpacing(6);
  horizontalLayout->addWidget(riftingHistoryTable_);

  QGridLayout* lithoGridLayout = new QGridLayout();
  lithoGridLayout->setSpacing(6);
  lithoGridLayout->addWidget(lithosphereParameterTable_,0,0);


  // Fastcauldron
  QGridLayout* fastCauldronGridLayout = new QGridLayout();
  fastCauldronGridLayout->setSpacing(6);
  QLabel *numProc = new QLabel("Number of Processors (for HPC jobs):", this);
  const QSize numProcSize = QSize(225, 30);
  numProc->setFixedSize(numProcSize);
  numProc->setLayoutDirection(Qt::LeftToRight);
  fastCauldronGridLayout->addWidget(numProc,0,0);
  //
  lineEditNumProc_->setFixedWidth(35);
  fastCauldronGridLayout->addWidget(lineEditNumProc_,0,1);
  QLabel *runMode = new QLabel("Cauldron Mode:", this);
  const QSize runModeSize = QSize(100, 30);
  runMode->setFixedSize(runModeSize);
  runMode->setLayoutDirection(Qt::RightToLeft);
  fastCauldronGridLayout->addWidget(runMode,1,0);
  //
  QStringList runModeLst = QStringList() << "Decompaction" << "Hydrostatic" << "Iteratively Coupled";
  fastcldrnRunModeComboBox_->insertItems(1, runModeLst);
  fastCauldronGridLayout->addWidget(fastcldrnRunModeComboBox_,1,1);
  //
  const QSize runBtnFastCldrnSize = QSize(140, 30);
  pushButtonRunFastCauldron_->setFixedSize(runBtnFastCldrnSize);
  pushButtonRunFastCauldron_->setLayoutDirection(Qt::RightToLeft);
  fastCauldronGridLayout->addWidget(pushButtonRunFastCauldron_,2,0,50,2);
  lithoGridLayout->addLayout(fastCauldronGridLayout,0,1);

  QVBoxLayout* verticalLayoutTab = new QVBoxLayout(this);
  verticalLayoutTab->setSpacing(6);
  verticalLayoutTab->addLayout(horizontalLayoutProject3D);
  //verticalLayoutTab->addWidget(lithosphereParameterTable_);
  verticalLayoutTab->addLayout(lithoGridLayout);
  verticalLayoutTab->addLayout(horizontalLayout);
  //verticalLayoutTab->addLayout(verticalLayoutOptionWidgets);
  verticalLayoutTab->addLayout(CTCrunGridLayout);
}

LithosphereParameterTable* CTCtab::lithosphereParameterTable() const
{
  return lithosphereParameterTable_;
}


RiftingHistoryTable* CTCtab::riftingHistoryTable() const
{
  return riftingHistoryTable_;
}

QLineEdit* CTCtab::lineEditProject3D() const
{
  return lineEditProject3D_;
}

const QPushButton* CTCtab::pushSelectProject3D() const
{
  return pushSelectProject3D_;
}

const QPushButton* CTCtab::pushButtonCTCrunCTC() const
{
  return pushButtonCTCrunCTC_;
}

const QPushButton* CTCtab::pushButtonCTCoutputMaps() const
{
  return pushButtonCTCoutputMaps_;
}

const QPushButton* CTCtab::pushButtonRunFastCauldron() const
{
  return pushButtonRunFastCauldron_;
}

const QPushButton* CTCtab::pushButtonExportCTCoutputMaps() const
{
  return pushButtonExportCTCoutputMaps_;
}

QLineEdit* CTCtab::lineEditNumProc() const
{
  return lineEditNumProc_;
}

const QComboBox* CTCtab::fastcldrnRunModeComboBox() const
{
  return fastcldrnRunModeComboBox_;
}

} // namespace ctcWizard
