#include "t2zTab.h"

#include "view/sharedComponents/emphasisbutton.h"
#include "view/sharedComponents/customtitle.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QVBoxLayout>

namespace casaWizard
{

namespace sac
{

namespace lithology
{

T2Ztab::T2Ztab(QWidget* parent) :
  QWidget(parent),
  pushButtonSACrunT2Z_{new EmphasisButton("Run Depth Conversion", this)},
  comboBoxReferenceSurface_{new QComboBox(this)},
  comboBoxProjectSelection_{new QComboBox(this)},
  spinBoxSubSampling_{new QSpinBox(this)},
  spinBoxNumberOfCPUs_{new QSpinBox(this)},
  comboBoxClusterSelection_{new QComboBox(this)},
  exportT2ZScenario_{new QPushButton("Export T2Z Scenario")},
  exportT2ZMapsToZycor_{new QPushButton("Export T2Z maps to Zycor")}
{
  QGridLayout* layout = new QGridLayout();
  layout->addWidget(new CustomTitle("Project Settings"), 0, 0);
  layout->addWidget(new QLabel("Project Selection", this), 1, 0);
  layout->addWidget(comboBoxProjectSelection_, 1, 1);
  layout->addWidget(new QLabel("Reference surface", this), 2, 0);
  layout->addWidget(comboBoxReferenceSurface_, 2, 1);
  layout->addWidget(new CustomTitle("Run Parameters"), 3, 0);
  spinBoxSubSampling()->setMinimum(1);
  spinBoxSubSampling_->setValue(1);
  layout->addWidget(new QLabel("Sub sampling", this), 4, 0);
  layout->addWidget(spinBoxSubSampling_, 4, 1);
  layout->addWidget(new QLabel("Number of CPUs", this), 5, 0);
  layout->addWidget(spinBoxNumberOfCPUs_, 5, 1);
  spinBoxNumberOfCPUs_->setMinimum(1);
  spinBoxNumberOfCPUs_->setMaximum(1024);
  layout->addWidget(new QLabel("Cluster", this), 6, 0);
  layout->addWidget(comboBoxClusterSelection_, 6, 1);
  comboBoxClusterSelection_->addItems({"LOCAL", "CLUSTER"});
  layout->addWidget(pushButtonSACrunT2Z_, 7, 1);
  layout->addWidget(exportT2ZScenario_, 8, 1);
  layout->addWidget(exportT2ZMapsToZycor_, 9, 1);

  layout->addWidget(new QWidget(this), 10, 2);
  layout->setRowStretch(10,1);
  layout->setColumnStretch(2,1);

  setLayout(layout);
}

const QPushButton* T2Ztab::pushButtonSACrunT2Z() const
{
  return pushButtonSACrunT2Z_;
}

const QPushButton *T2Ztab::exportT2ZScenario() const
{
  return exportT2ZScenario_;
}

const QPushButton*T2Ztab::exportT2ZMapsToZycor() const
{
  return exportT2ZMapsToZycor_;
}

QComboBox* T2Ztab::comboBoxProjectSelection() const
{
  return comboBoxProjectSelection_;
}

QComboBox* T2Ztab::comboBoxReferenceSurface() const
{
  return comboBoxReferenceSurface_;
}

QSpinBox* T2Ztab::spinBoxSubSampling() const
{
  return spinBoxSubSampling_;
}

QSpinBox* T2Ztab::spinBoxNumberOfCPUs() const
{
  return spinBoxNumberOfCPUs_;
}

QComboBox*T2Ztab::comboBoxClusterSelection() const
{
  return comboBoxClusterSelection_;
}

void T2Ztab::addProjectSelectionOptions(const QStringList& options)
{
  comboBoxProjectSelection_->clear();
  comboBoxProjectSelection_->addItems(options);
}

void T2Ztab::setReferenceSurfaces(const QStringList& surfaces)
{
  comboBoxReferenceSurface_->clear();
  comboBoxReferenceSurface_->addItems(surfaces);
}

bool T2Ztab::noProjectAvailable() const
{
  return comboBoxProjectSelection_->count() == 0;
}

} // namespace lithology

} // namespace sac

} // namespace casaWizard
