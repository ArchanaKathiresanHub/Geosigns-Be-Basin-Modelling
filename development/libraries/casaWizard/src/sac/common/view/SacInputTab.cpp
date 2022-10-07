//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "SacInputTab.h"

#include "view/calibrationTargetTable.h"
#include "view/assets/objectiveFunctionTableSAC.h"
#include "view/sharedComponents/customtitle.h"
#include "view/sharedComponents/emphasisbutton.h"
#include "view/sharedComponents/helpLabel.h"

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

SacInputTab::SacInputTab(QWidget* parent) :
   QWidget(parent),
   m_subLayoutActivationWidget{new QWidget(this)},
   m_pushSelectProject3D{new QPushButton("Select", this)},
   m_lineEditProject3D{new QLineEdit(this)},
   m_layoutTablesAndOptions{new QGridLayout(m_subLayoutActivationWidget)},
   m_layoutRunOptions{new QVBoxLayout()},
   m_calibrationTargetTable{new CalibrationTargetTable(m_subLayoutActivationWidget)},
   m_objectiveFunctionTable{new ObjectiveFunctionTableSAC(m_subLayoutActivationWidget)},
   m_pushSelectAllWells{new QPushButton("Select all", m_subLayoutActivationWidget)},
   m_pushClearSelection{new QPushButton("Deselect all", m_subLayoutActivationWidget)},
   m_comboBoxApplication{new QComboBox(m_subLayoutActivationWidget)},
   m_comboBoxCluster{new QComboBox(m_subLayoutActivationWidget)},
   m_pushButtonRun1DOptimalization{new EmphasisButton("Run 1D optimization", m_subLayoutActivationWidget)},
   m_buttonRunOriginal1D{new QPushButton("Run original 1D", m_subLayoutActivationWidget)},
   m_buttonRunOriginal3D{new QPushButton("Run original 3D", m_subLayoutActivationWidget)}
{
   m_comboBoxApplication->insertItems(0, {"Iteratively Coupled", "Hydrostatic"});
   m_comboBoxCluster->insertItems(0, {"LOCAL", "CLUSTER"});

   QHBoxLayout* layoutProject3D = new QHBoxLayout();
   layoutProject3D->addWidget(new QLabel("Project file", this));
   layoutProject3D->addWidget(m_lineEditProject3D);
   layoutProject3D->addWidget(m_pushSelectProject3D);
   layoutProject3D->setContentsMargins(11, 0, 11, 0);

   m_lineEditProject3D->setReadOnly(true);

   m_layoutCalibrationOptions = new QHBoxLayout();
   m_layoutCalibrationOptions->addWidget(new CustomTitle("Calibration Targets"));
   m_layoutCalibrationOptions->addWidget(m_pushSelectAllWells);
   m_layoutCalibrationOptions->addWidget(m_pushClearSelection);
   m_layoutCalibrationOptions->setStretch(0,4);

   QGridLayout* layoutOptions = new QGridLayout();
   layoutOptions->addWidget(new QLabel("Run Mode", m_subLayoutActivationWidget), 1, 0);
   layoutOptions->addWidget(m_comboBoxApplication, 1, 1);
   layoutOptions->addWidget(new QLabel("Run Location", m_subLayoutActivationWidget), 2, 0);
   layoutOptions->addWidget(m_comboBoxCluster, 2, 1);
   layoutOptions->addWidget(m_pushButtonRun1DOptimalization, 3, 0, 1, 2);

   QHBoxLayout* run1D = new QHBoxLayout();
   run1D->addWidget(m_buttonRunOriginal1D);
   HelpLabel* helpLabelRun1D = new HelpLabel(m_subLayoutActivationWidget, "For plotting and QC purposes under 'Well log plots and Results' tab");
   run1D->addWidget(helpLabelRun1D);
   layoutOptions->addLayout(run1D, 4, 0, 1, 2);

   QHBoxLayout* run3D = new QHBoxLayout();
   run3D->addWidget(m_buttonRunOriginal3D);
   HelpLabel* helpLabelRun3D = new HelpLabel(m_subLayoutActivationWidget, "For plotting and QC purposes under 'Well log plots and Results' tab");
   run3D->addWidget(helpLabelRun3D);
   layoutOptions->addLayout(run3D, 5, 0, 1, 2);

   layoutOptions->setMargin(0);
   m_layoutRunOptions->addLayout(layoutOptions);

   m_layoutTablesAndOptions->addLayout(m_layoutCalibrationOptions, 1, 0);
   m_layoutTablesAndOptions->addWidget(m_calibrationTargetTable, 2,0);

   m_layoutTablesAndOptions->addWidget(new CustomTitle("Data series and uncertainty ranges"), 1, 1);
   m_objectiveFunctionTable->setMinimumWidth(600);
   m_layoutTablesAndOptions->addWidget(m_objectiveFunctionTable, 2, 1);

   m_layoutTablesAndOptions->addWidget(new CustomTitle("Run Options"), 1,2);
   m_layoutTablesAndOptions->addLayout(m_layoutRunOptions, 2, 2, Qt::Alignment(Qt::AlignmentFlag::AlignTop));

   m_layoutTablesAndOptions->setColumnStretch(0, 3);
   m_layoutTablesAndOptions->setColumnStretch(1, 2);
   m_layoutTablesAndOptions->setColumnStretch(2, 0);

   QVBoxLayout* verticalLayoutTab = new QVBoxLayout(this);
   verticalLayoutTab->addLayout(layoutProject3D);
   verticalLayoutTab->addWidget(m_subLayoutActivationWidget,5);
   m_subLayoutActivationWidget->setEnabled(false);
}

void SacInputTab::setContentsActive(bool state)
{
   m_subLayoutActivationWidget->setEnabled(state);
}

void SacInputTab::addWidget(QWidget* widget, QString title)
{
   QVBoxLayout* layout = new QVBoxLayout();
   layout->addWidget(new CustomTitle(title));
   layout->addWidget(widget);
   m_layoutTablesAndOptions->addLayout(layout,0,0,1,3);
}

QHBoxLayout* SacInputTab::layoutCalibrationOptions() const
{
   return m_layoutCalibrationOptions;
}

QVBoxLayout* SacInputTab::layoutRunOptions() const
{
   return m_layoutRunOptions;
}

CalibrationTargetTable* SacInputTab::calibrationTargetTable() const
{
   return m_calibrationTargetTable;
}

ObjectiveFunctionTableSAC* SacInputTab::objectiveFunctionTable() const
{
   return m_objectiveFunctionTable;
}

QLineEdit* SacInputTab::lineEditProject3D() const
{
   return m_lineEditProject3D;
}

const QPushButton* SacInputTab::pushSelectProject3D() const
{
   return m_pushSelectProject3D;
}

QComboBox* SacInputTab::comboBoxApplication() const
{
   return m_comboBoxApplication;
}

QComboBox* SacInputTab::comboBoxCluster() const
{
   return m_comboBoxCluster;
}

const QPushButton*SacInputTab::pushSelectAllWells() const
{
   return m_pushSelectAllWells;
}

const QPushButton*SacInputTab::pushClearSelection() const
{
   return m_pushClearSelection;
}

const QPushButton* SacInputTab::pushRun1DOptimalization() const
{
   return m_pushButtonRun1DOptimalization;
}

const QPushButton* SacInputTab::buttonRunOriginal1D() const
{
   return m_buttonRunOriginal1D;
}

const QPushButton* SacInputTab::buttonRunOriginal3D() const
{
   return m_buttonRunOriginal3D;
}

} // namespace sac

} // namespace casaWizard
