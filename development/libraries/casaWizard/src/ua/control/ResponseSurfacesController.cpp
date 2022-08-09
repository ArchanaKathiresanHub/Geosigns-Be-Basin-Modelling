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
  m_responseSurfacesTab{responseSurfacesTab},
  m_casaScenario{casaScenario},
  m_scriptRunController{scriptRunController},
  m_qcDoeOptionController{new QCDoeOptionController(m_responseSurfacesTab->qcDoeOptionTable(), m_casaScenario, this)}
{
  connect(parent, SIGNAL(signalUpdateTabGUI(int)), this, SLOT(slotUpdateTabGUI(int)));

  connect(m_responseSurfacesTab->pushButtonQCrunCASA(), SIGNAL(clicked()),
          this,                          SLOT(slotPushButtonQCrunCasaClicked()));
  connect(m_responseSurfacesTab->tableQC(),             SIGNAL(currentItemChanged(QTableWidgetItem*, QTableWidgetItem*)),
          this,                          SLOT(slotTableQCCurrentItemChanged(QTableWidgetItem*, QTableWidgetItem*)));

  connect(m_qcDoeOptionController, SIGNAL(modelChange()), this, SLOT(slotModelChange()));
}

void ResponseSurfacesController::slotModelChange()
{
  m_casaScenario.setStageComplete(StageTypesUA::responseSurfaces, false);
  m_casaScenario.setStageComplete(StageTypesUA::mcmc, false);

  m_casaScenario.setTargetQCs({});
  m_casaScenario.monteCarloDataManager().clear();

  refreshGUI();
}

void ResponseSurfacesController::refreshGUI()
{
  m_responseSurfacesTab->fillQCtable(m_casaScenario.targetQCs());
  m_responseSurfacesTab->updateQCPlot({});

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

  if (!m_casaScenario.isStageComplete(StageTypesUA::doe))
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
    const RunCaseSetFileManager& runCaseSetFileManager = m_casaScenario.runCaseSetFileManager();
    if (runCaseSetFileManager.isIterationDirDeleted(m_casaScenario.project3dPath()))
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

  const QString source = m_casaScenario.workingDirectory() + "/" + m_casaScenario.stateFileNameQC() ;
  const QString target = m_casaScenario.workingDirectory() + "/" + m_casaScenario.runLocation() + "/" + m_casaScenario.iterationDirName() + "/" + m_casaScenario.stateFileNameQC();

  if (QFile::exists(target))
  {
    QFile::remove(target);
  }

  if (QFile::copy(source, target))
  {
    QFile::remove(source);
  }

  m_casaScenario.setStageComplete(StageTypesUA::responseSurfaces);
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
  int row = current->row();

  m_responseSurfacesTab->updateQCPlot(m_casaScenario.targetQCs()[row]);
  Logger::log() << "Displaying target " << m_casaScenario.targetQCs()[row].id() << Logger::endl();
}

} // namespace ua

} // namespace casaWizard
