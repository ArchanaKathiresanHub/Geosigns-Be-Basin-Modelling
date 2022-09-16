#include "ModelInputsTab.h"

#include "doeOptionTable.h"
#include "influentialParameterTable.h"
#include "manualDesignPointTable.h"
#include "model/doeOption.h"
#include "view/components/customtitle.h"

#include <QComboBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QLayout>

namespace casaWizard
{

namespace ua
{

ModelInputsTab::ModelInputsTab(QWidget* parent) :
  QWidget(parent),
  pushSelectProject3D_{new QPushButton("Select", this)},
  lineEditProject3D_{new QLineEdit(this)},
  doeOptionTable_{new DoeOptionTable(this)},
  influentialParameterTable_{new InfluentialParameterTable(this)},
  manualDesignPointTable_{new ManualDesignPointTable(this)},
  comboBoxApplication_{new QComboBox(this)},
  comboBoxCluster_{new QComboBox(this)},
  spinBoxCPUs_{new QSpinBox(this)},
  m_spinBoxSubSampling{new QSpinBox(this)},
  pushButtonDoErunCASA_{new QPushButton("Run DoE points", this)},
  pushButtonRunAddedCases_{new QPushButton("Run added points", this)}
{
  comboBoxApplication_->insertItems(0, QStringList() << "fastcauldron \"-itcoupled\"" << "fastcauldron \"-temperature\"");
  comboBoxCluster_->insertItems(0, QStringList() << "LOCAL" << "CLUSTER");
  spinBoxCPUs_->setMinimum(1);
  spinBoxCPUs_->setMaximum(256);

  QHBoxLayout* layoutProject3D = new QHBoxLayout();
  layoutProject3D->addWidget(new QLabel("Project file", this));
  layoutProject3D->addWidget(lineEditProject3D_);
  layoutProject3D->addWidget(pushSelectProject3D_);

  QFormLayout* layoutOptions = new QFormLayout();
  layoutOptions->addRow(new QLabel("Application", this), comboBoxApplication_);
  layoutOptions->addRow(new QLabel("Cluster", this), comboBoxCluster_);
  layoutOptions->addRow(new QLabel("Number of processors", this), spinBoxCPUs_);
  m_spinBoxSubSampling->setMinimum(1);
  m_spinBoxSubSampling->setValue(1);
  layoutOptions->addRow(new QLabel("Sub sampling", this), m_spinBoxSubSampling);

  layoutOptions->addRow(new QWidget(this), pushButtonDoErunCASA_);
  layoutOptions->addRow(new QWidget(this), pushButtonRunAddedCases_);

  QVBoxLayout* layoutDoe = new QVBoxLayout();
  layoutDoe->addWidget(new CustomTitle("Design of Experiments", this));
  layoutDoe->addWidget(doeOptionTable_);

  QVBoxLayout* layoutPoints = new QVBoxLayout();
  layoutPoints->addWidget(new CustomTitle("Manual design points", this));
  layoutPoints->addWidget(manualDesignPointTable_);

  QHBoxLayout* layoutDoeAndOptions = new QHBoxLayout();
  layoutDoeAndOptions->addLayout(layoutDoe);
  layoutDoeAndOptions->addLayout(layoutPoints, 1);
  layoutDoeAndOptions->addLayout(layoutOptions);

  QVBoxLayout* verticalLayoutTab = new QVBoxLayout(this);
  verticalLayoutTab->addLayout(layoutProject3D);
  verticalLayoutTab->addWidget(influentialParameterTable_);
  verticalLayoutTab->addLayout(layoutDoeAndOptions);
}

InfluentialParameterTable* ModelInputsTab::influentialParameterTable() const
{
  return influentialParameterTable_;
}

ManualDesignPointTable* ModelInputsTab::manualDesignPointTable() const
{
  return manualDesignPointTable_;
}

QLineEdit* ModelInputsTab::lineEditProject3D() const
{
  return lineEditProject3D_;
}

const QPushButton* ModelInputsTab::pushSelectProject3D() const
{
  return pushSelectProject3D_;
}

QComboBox* ModelInputsTab::comboBoxApplication() const
{
  return comboBoxApplication_;
}

QComboBox* ModelInputsTab::comboBoxCluster() const
{
  return comboBoxCluster_;
}

QSpinBox* ModelInputsTab::spinBoxCPUs() const
{
   return spinBoxCPUs_;
}

QSpinBox* ModelInputsTab::spinBoxSubSampling() const
{
   return m_spinBoxSubSampling;
}

QPushButton* ModelInputsTab::pushButtonDoeRunCASA() const
{
  return pushButtonDoErunCASA_;
}

QPushButton*ModelInputsTab::pushButtonRunAddedCases() const
{
  return pushButtonRunAddedCases_;
}

void ModelInputsTab::updateDoeOptionTable(const QVector<DoeOption*>& doeOptions, const QVector<bool>& isDoeOptionsSelected)
{
  doeOptionTable_->updateTable(doeOptions, isDoeOptionsSelected);
}

QTableWidget* ModelInputsTab::doeOptionTable() const
{
  return doeOptionTable_;
}

int ModelInputsTab::columnIndexCheckBoxDoeOptionTable() const
{
  return doeOptionTable_->columnCheckBoxDoeOptionTable();
}

int ModelInputsTab::columnIndexNDesignPointsDoeOptionTable() const
{
  return doeOptionTable_->columnIndexNDesignPointsDoeOptionTable();
}

} // namespace ua

} // namespace casaWizard
