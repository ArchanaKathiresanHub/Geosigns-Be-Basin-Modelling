//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "correlationController.h"

#include "control/ScriptRunController.h"
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
   connect(parent, SIGNAL(signalUpdateTabGUI(int)), this, SLOT(slotUpdateTabGUI(int)));

   connect(correlationTab_->tableParameters(), SIGNAL(itemChanged(QTableWidgetItem*)),
           this,
           SLOT(correlationSelectionItemChanged()));
}

void CorrelationController::refreshGUI()
{
   const InfluentialParameterManager& manager{casaScenario_.influentialParameterManager()};

   if (casaScenario_.isStageComplete(StageTypesUA::mcmc))
   {
      correlationTab_->fillCorrelationSelectionTable(manager.influentialParameters(), manager.isUsedInCorrelation());
      correlationTab_->colorbar()->setVisible(true);
      updateCorrelationPlotLayout();
   }
   else
   {
      correlationTab_->fillCorrelationSelectionTable({},{});
      correlationTab_->updateCorrelationPlotLayout({},{},{},{});
   }
}

void CorrelationController::slotUpdateTabGUI(int tabID)
{
   if (tabID != static_cast<int>(TabID::Correlations))
   {
      return;
   }

   correlationTab_->setEnabled(true);
   if (!casaScenario_.isStageComplete(StageTypesUA::mcmc))
   {
      correlationTab_->setEnabled(false);
      correlationTab_->colorbar()->setVisible(false);
      Logger::log() << "MCMC data is not available! Complete MCMC data creation stage in MCMC tab first." << Logger::endl();
   }

   refreshGUI();
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

} // namespace ua

} // namespace casaWizard
