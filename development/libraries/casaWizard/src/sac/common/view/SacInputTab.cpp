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
   m_pushSelectProject3D{new QPushButton("Select", this)},
   m_lineEditProject3D{new QLineEdit(this)},
   m_layoutTablesAndOptions{new QGridLayout()},
   m_calibrationTargetTable{new CalibrationTargetTable(this)},
   m_objectiveFunctionTable{new ObjectiveFunctionTableSAC(this)},
   m_pushSelectAllWells{new QPushButton("Select all", this)},
   m_pushClearSelection{new QPushButton("Deselect all", this)},
   m_comboBoxApplication{new QComboBox(this)},
   m_comboBoxCluster{new QComboBox(this)},
   m_pushButtonRun1DOptimalization{new EmphasisButton("Run 1D optimization", this)},
   m_buttonRunOriginal1D{new QPushButton("Run original 1D", this)},
   m_buttonRunOriginal3D{new QPushButton("Run original 3D", this)}
{
   m_comboBoxApplication->insertItems(0, {"Iteratively Coupled", "Hydrostatic"});
   m_comboBoxCluster->insertItems(0, {"LOCAL", "CLUSTER"});

   QHBoxLayout* layoutProject3D = new QHBoxLayout();
   layoutProject3D->addWidget(new QLabel("Project file", this));
   layoutProject3D->addWidget(m_lineEditProject3D);
   layoutProject3D->addWidget(m_pushSelectProject3D);

   m_lineEditProject3D->setReadOnly(true);

   m_layoutCalibrationOptions = new QHBoxLayout();
   m_layoutCalibrationOptions->addWidget(new CustomTitle("Calibration Targets"));
   m_layoutCalibrationOptions->addWidget(m_pushSelectAllWells);
   m_layoutCalibrationOptions->addWidget(m_pushClearSelection);
   m_layoutCalibrationOptions->setStretch(0,4);

   QWidget* runOptions = new QWidget(this);
   QGridLayout* layoutOption = new QGridLayout(runOptions);
   layoutOption->addWidget(new CustomTitle("Run Options", this), 0, 0);
   layoutOption->addWidget(new QLabel("Run Mode", this), 1, 0);
   layoutOption->addWidget(m_comboBoxApplication, 1, 1);
   layoutOption->addWidget(new QLabel("Run Location", this), 2, 0);
   layoutOption->addWidget(m_comboBoxCluster, 2, 1);

   layoutOption->addWidget(m_pushButtonRun1DOptimalization, 3, 0, 1, 2);

   QHBoxLayout* run1D = new QHBoxLayout();
   run1D->addWidget(m_buttonRunOriginal1D);
   HelpLabel* helpLabelRun1D = new HelpLabel(this, "For plotting and QC purposes under 'Well log plots and Results' tab");
   run1D->addWidget(helpLabelRun1D);
   layoutOption->addLayout(run1D, 4, 0, 1, 2);

   QHBoxLayout* run3D = new QHBoxLayout();
   run3D->addWidget(m_buttonRunOriginal3D);
   HelpLabel* helpLabelRun3D = new HelpLabel(this, "For plotting and QC purposes under 'Well log plots and Results' tab");
   run3D->addWidget(helpLabelRun3D);
   layoutOption->addLayout(run3D, 5, 0, 1, 2);
   layoutOption->setMargin(0);

   runOptions->setMaximumHeight(160);

   QVBoxLayout* calibrationTargetTableLayout = new QVBoxLayout();
   calibrationTargetTableLayout->addLayout(m_layoutCalibrationOptions);
   calibrationTargetTableLayout->addWidget(m_calibrationTargetTable);

   m_layoutTablesAndOptions->addLayout(calibrationTargetTableLayout, 1,0);

   QVBoxLayout* objectiveFunctionLayout = new QVBoxLayout();

   objectiveFunctionLayout->addWidget(new CustomTitle("Data series and uncertainty ranges"));
   objectiveFunctionLayout->addWidget(m_objectiveFunctionTable);

   m_layoutTablesAndOptions->addLayout(objectiveFunctionLayout,1,1);
   m_layoutTablesAndOptions->addWidget(runOptions,1,2, Qt::Alignment(Qt::AlignmentFlag::AlignTop));

   QVBoxLayout* verticalLayoutTab = new QVBoxLayout(this);
   verticalLayoutTab->addLayout(layoutProject3D);
   verticalLayoutTab->addLayout(m_layoutTablesAndOptions);
}

void SacInputTab::addTable(QTableWidget* table, QString tableTitle)
{
   QVBoxLayout* layout = new QVBoxLayout();
   layout->addWidget(new CustomTitle(tableTitle));
   layout->addWidget(table);
   m_layoutTablesAndOptions->addLayout(layout,0,0,1,3);
}

void SacInputTab::addImportButton(QPushButton* button)
{
   m_layoutCalibrationOptions->insertWidget(1, button);
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
