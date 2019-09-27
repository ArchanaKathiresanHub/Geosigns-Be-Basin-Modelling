#include "correlationController.h"

#include "control/scriptRunController.h"
#include "model/influentialParameterManager.h"
#include "model/logger.h"
#include "model/uaScenario.h"
#include "model/targetQC.h"
#include "view/correlationTab.h"
#include "view/uaTabIDs.h"

#include <QComboBox>
#include <QPushButton>
#include <QStringList>
#include <QTableWidget>

namespace casaWizard
{

namespace ua
{

CorrelationController::CorrelationController(CorrelationTab* correlationTab,
                                             UAScenario& casaScenario,
                                             ScriptRunController& scriptRunController,
                                             QObject* parent) :
  QObject(parent),
  correlationTab_{correlationTab},
  casaScenario_(casaScenario),
  scriptRunController_{scriptRunController}
{
  connect(correlationTab_->tableParameters(), SIGNAL(itemChanged(QTableWidgetItem*)),
          this,
          SLOT(correlationSelectionItemChanged()));
}

void CorrelationController::slotRefresh(int tabID)
{
  if (tabID != static_cast<int>(TabID::Correlations))
  {
    return;
  }

  const InfluentialParameterManager& manager{casaScenario_.influentialParameterManager()};
  correlationTab_->fillCorrelationSelectionTable(manager.influentialParameters(), manager.isUsedInCorrelation());
  updateCorrelationPlotLayout();
}

void CorrelationController::slotEnableDisableDependentWorkflowTabs(int tabID, bool hasLogMessage)
{
  if (tabID != static_cast<int>(TabID::Correlations) || correlationTab_->isEnabled())
  {
    return;
  }

  if (hasLogMessage)
  {
    Logger::log() << "MCMC data is not available! Complete MCMC data creation stage in UA tab first." << Logger::endl();
  }
}

void CorrelationController::updateCorrelationPlotLayout()
{
  const MonteCarloDataManager& mcData{casaScenario_.monteCarloDataManager()};
  const InfluentialParameterManager& manager{casaScenario_.influentialParameterManager()};
  correlationTab_->updateCorrelationPlotLayout(casaScenario_.influentialParametersWithRunData(), manager.isUsedInCorrelation(),
                                               mcData.influentialParameterMatrix(), mcData.rmse());
}

void CorrelationController::correlationSelectionItemChanged()
{
  InfluentialParameterManager& manager{casaScenario_.influentialParameterManager()};
  manager.setIsUsedInCorrelation(correlationTab_->isCorrelationSelectTableItemSelected());
  updateCorrelationPlotLayout();
}

void CorrelationController::slotEnableTabAndUpdateDependentWorkflowTabs()
{
  correlationTab_->setEnabled(true);
}

void CorrelationController::slotDisableTabAndUpdateDependentWorkflowTabs()
{
  correlationTab_->setEnabled(false);
}

} // namespace ua

} // namespace casaWizard
