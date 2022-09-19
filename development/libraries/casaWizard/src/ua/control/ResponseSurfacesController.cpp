#include "ResponseSurfacesController.h"

#include "control/casaScriptWriter.h"
#include "control/scriptRunController.h"
#include "model/input/targetQCdataCreator.h"
#include "model/logger.h"
#include "model/scenarioBackup.h"
#include "model/script/qcScript.h"
#include "model/stagesUA.h"
#include "model/targetQC.h"
#include "model/uaScenario.h"
#include "qcDoeOptionController.h"
#include "view/ResponseSurfacesTab.h"
#include "view/uaTabIDs.h"

#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QStringList>
#include <QTableWidget>

namespace casaWizard
{

namespace ua
{

ResponseSurfacesController::ResponseSurfacesController(ResponseSurfacesTab* responseSurfacesTab,
                           UAScenario& casaScenario,
                           ScriptRunController& scriptRunController,
                           QObject* parent) :
  QObject(parent),
  m_targetIndex{0},
  m_responseSurfacesTab{responseSurfacesTab},
  m_casaScenario{casaScenario},
  m_scriptRunController{scriptRunController},
  m_qcDoeOptionController{new QCDoeOptionController(m_responseSurfacesTab->qcDoeOptionTable(), casaScenario, this)}
{
  connect(parent, SIGNAL(signalUpdateTabGUI(int)), this, SLOT(slotUpdateTabGUI(int)));

  connect(m_responseSurfacesTab->pushButtonQCrunCASA(), SIGNAL(clicked()),
          this,                          SLOT(slotPushButtonQCrunCasaClicked()));
  connect(m_responseSurfacesTab->tableQC(),             SIGNAL(currentItemChanged(QTableWidgetItem*, QTableWidgetItem*)),
          this,                          SLOT(slotTableQCCurrentItemChanged(QTableWidgetItem*, QTableWidgetItem*)));

  connect(m_qcDoeOptionController, SIGNAL(modelChange()), this, SLOT(slotModelChange()));

  refreshGUI();
}

void ResponseSurfacesController::slotModelChange()
{
  m_casaScenario.setStageComplete(StageTypesUA::responseSurfaces, false);
  m_casaScenario.setStageComplete(StageTypesUA::mcmc, false);
  m_targetIndex = 0;

  m_casaScenario.setTargetQCs({});
  m_casaScenario.monteCarloDataManager().clear();

  refreshGUI();
}

void ResponseSurfacesController::refreshGUI()
{
  m_responseSurfacesTab->fillQCtable(m_casaScenario.targetQCs());
  if (m_casaScenario.targetQCs().size() > 0) {
     m_responseSurfacesTab->updateQCPlot( m_casaScenario.targetQCs().at(m_targetIndex), m_targetIndex);
  }
  emit signalRefreshChildWidgets();
}

void ResponseSurfacesController::slotUpdateTabGUI(int tabID)
{
  if (tabID != static_cast<int>(TabID::QC))
  {
    return;
  }

  m_responseSurfacesTab->allowModification();
  m_responseSurfacesTab->setEnabled(true);

  if (!m_casaScenario.isStageComplete(StageTypesUA::doe) || !m_casaScenario.isStageUpToDate(StageTypesUA::doe))
  {
    m_responseSurfacesTab->setEnabled(false);
    Logger::log() << "DoE data is not available! Complete DoE stage in DoE tab first." << Logger::endl();
  }
  else if (!m_casaScenario.isStageComplete(StageTypesUA::responseSurfaces))
  {
    slotModelChange();
    Logger::log() << "QC stage is not completed! Run CASA to complete it." << Logger::endl();
  }
  else
  {
    if (!m_casaScenario.iterationDirExists())
    {
      m_responseSurfacesTab->allowModification(false);
    }
  }

  refreshGUI();
}

void ResponseSurfacesController::slotPushButtonQCrunCasaClicked()
{
  scenarioBackup::backup(m_casaScenario);

  const CalibrationTargetManager& ctManager = m_casaScenario.calibrationTargetManager();
  const PredictionTargetManager& ptManager = m_casaScenario.predictionTargetManager();
  if ( (ctManager.amountOfActiveCalibrationTargets() + ptManager.amountAtAge0()) < 1)
  {
    Logger::log() << "There are no active targets. Either provide prediction targets or active/import wells. No response surfaces to calculate." << Logger::endl();
    return;
  }

  QStringList selectedDoes = m_casaScenario.qcDoeOptionSelectedNames();
  if (selectedDoes.isEmpty())
  {
    Logger::log() << "No DOE selected for quality check calculation." << Logger::endl();
    return;
  }

  QCScript qc{m_casaScenario};
  if (!casaScriptWriter::writeCasaScript(qc) ||
      !m_scriptRunController.runScript(qc))
  {
    return;
  }

  try
  {
    targetQCdataCreator::readTargetQCs(m_casaScenario);
    m_responseSurfacesTab->fillQCtable(m_casaScenario.targetQCs());
  }
  catch (const std::exception& e)
  {
    Logger::log() << "Failed to read the targets for the quality check: " << e.what() << Logger::endl();
  }

  m_casaScenario.copyToIterationDir(m_casaScenario.stateFileNameQC());

  m_casaScenario.setStageComplete(StageTypesUA::responseSurfaces);
  m_casaScenario.setStageUpToDate(StageTypesUA::responseSurfaces);
  m_casaScenario.setStageComplete(StageTypesUA::mcmc, false);

  scenarioBackup::backup(m_casaScenario);

  slotUpdateTabGUI(static_cast<int>(TabID::QC));
}

void ResponseSurfacesController::slotTableQCCurrentItemChanged(QTableWidgetItem* current, QTableWidgetItem* /*previous*/)
{
  if (current == nullptr)
  {
    return;
  }
  m_targetIndex = current->row();

  m_responseSurfacesTab->updateQCPlot(m_casaScenario.targetQCs()[m_targetIndex], m_targetIndex);
  Logger::log() << "Displaying target " << m_casaScenario.targetQCs()[m_targetIndex].id() << Logger::endl();
}

} // namespace ua

} // namespace casaWizard
